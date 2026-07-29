#include "gridx/matching/adapters/kafka/KafkaGridTransferConsumer.hpp"

#include "gridx/matching/adapters/kafka/GridTransferEventMapper.hpp"
#include "gridx/matching/adapters/kafka/KafkaConsumerConfig.hpp"
#include "gridx/matching/adapters/kafka/ProtobufCodec.hpp"
#include "gridx/matching/domain/GridTransferRule.hpp"

#include <librdkafka/rdkafkacpp.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <charconv>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

namespace gridx::matching::adapters::kafka {

namespace {

using PartitionOffsetMap = std::unordered_map<std::int32_t, std::int64_t>;

struct ParsedGridTransferKey {
    GridZoneId sellerGridZone{};
    GridZoneId buyerGridZone{};
};

// Parses seller:buyer string to GridZoneId
[[nodiscard]]
std::optional<ParsedGridTransferKey> parseGridTransferKey(const std::string_view key) {
    const auto separator = key.find(':');

    if (separator == std::string_view::npos || separator == 0 || separator == key.size() - 1 ||
        key.find(':', separator + 1) != std::string_view::npos) {
        return std::nullopt;
    }

    const auto sellerPart = key.substr(0, separator);
    const auto buyerPart = key.substr(separator + 1);

    GridZoneId sellerGridZone{};
    GridZoneId buyerGridZone{};

    const auto sellerResult =
        std::from_chars(sellerPart.data(), sellerPart.data() + sellerPart.size(), sellerGridZone);

    const auto buyerResult =
        std::from_chars(buyerPart.data(), buyerPart.data() + buyerPart.size(), buyerGridZone);

    if (sellerResult.ec != std::errc{} ||
        sellerResult.ptr != sellerPart.data() + sellerPart.size() ||
        buyerResult.ec != std::errc{} || buyerResult.ptr != buyerPart.data() + buyerPart.size()) {
        return std::nullopt;
    }

    return ParsedGridTransferKey{
        .sellerGridZone = sellerGridZone,
        .buyerGridZone = buyerGridZone,
    };
}

// Checks all partitions reached 
// required offsets (targetOffsets ≤ consumedPositions)
[[nodiscard]]
bool allBootstrapTargetsReached(
    const PartitionOffsetMap& targetOffsets,
    const PartitionOffsetMap& consumedPositions
) {
    for (const auto& [partition, targetOffset] : targetOffsets) {
        const auto position = consumedPositions.find(partition);

        if (position == consumedPositions.end() || position->second < targetOffset) {
            return false;
        }
    }

    return true;
}

void destroyTopicPartitions(std::vector<RdKafka::TopicPartition*>& partitions) noexcept {
    for (auto* partition : partitions) {
        delete partition;
    }

    partitions.clear();
}

[[nodiscard]]
std::vector<std::int32_t> discoverPartitions(
    RdKafka::KafkaConsumer& consumer,
    const std::string& topic,
    const int timeoutMs
) {
    RdKafka::Metadata* rawMetadata = nullptr;

    const auto error = consumer.metadata(false, nullptr, &rawMetadata, timeoutMs);

    std::unique_ptr<RdKafka::Metadata> metadata{rawMetadata};

    if (error != RdKafka::ERR_NO_ERROR || !metadata) {
        throw std::runtime_error{
            "Failed to read Kafka metadata for topic '" + topic +
            "': " + RdKafka::err2str(error)
        };
    }

    const RdKafka::TopicMetadata* selectedTopic = nullptr;

    for (const auto* topicMetadata : *metadata->topics()) {
        if (topicMetadata != nullptr && topicMetadata->topic() == topic) {
            selectedTopic = topicMetadata;
            break;
        }
    }

    if (selectedTopic == nullptr) {
        throw std::runtime_error{"Kafka topic metadata was not found for '" + topic + "'"};
    }

    if (selectedTopic->err() != RdKafka::ERR_NO_ERROR) {
        throw std::runtime_error{
            "Kafka topic '" + topic + "' is unavailable: " + RdKafka::err2str(selectedTopic->err())
        };
    }

    std::vector<std::int32_t> partitions;
    partitions.reserve(selectedTopic->partitions()->size());

    for (const auto* partitionMetadata : *selectedTopic->partitions()) {
        if (partitionMetadata != nullptr) {
            partitions.push_back(partitionMetadata->id());
        }
    }

    if (partitions.empty()) {
        throw std::runtime_error{"Kafka topic '" + topic + "' has no partitions"};
    }

    return partitions;
}

}  // namespace

KafkaGridTransferConsumer::KafkaGridTransferConsumer(
    KafkaConsumerConfig config,
    config::GridTransferCache& cache
) : config_{std::move(config)}, cache_{cache} {}

KafkaGridTransferConsumer::~KafkaGridTransferConsumer() {
    stop();
}

void KafkaGridTransferConsumer::initializeConsumer() {
    if (consumer_) {
        return;
    }

    std::string errorMessage;

    std::unique_ptr<RdKafka::Conf> globalConfig{RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL)};

    if (!globalConfig) {
        throw std::runtime_error{
            "Failed to create librdkafka global "
            "configuration"};
    }

    const auto setConfig = [&](const std::string& name, const std::string& value) {
        if (globalConfig->set(name, value, errorMessage) != RdKafka::Conf::CONF_OK) {
            throw std::runtime_error{
                "Failed to set Kafka configuration '" + name + "': " + errorMessage
            };
        }
    };

    setConfig("bootstrap.servers", config_.bootstrapServers);

    setConfig("group.id", config_.consumerGroup);

    setConfig("enable.auto.commit", "false");
    setConfig("enable.auto.offset.store", "false");
    setConfig("enable.partition.eof", "true");
    setConfig("auto.offset.reset", "earliest");

    consumer_.reset(RdKafka::KafkaConsumer::create(globalConfig.get(), errorMessage));

    if (!consumer_) {
        throw std::runtime_error{
            "Failed to create Kafka grid-transfer "
            "consumer: " +
            errorMessage};
    }

    spdlog::info(
        "Created grid-transfer Kafka consumer "
        "for topic '{}'",
        config_.topic);
}

void KafkaGridTransferConsumer::start() {
    if (cache_.isReady()) {
        throw std::logic_error{
            "GridTransferCache has already been "
            "bootstrapped"};
    }

    initializeConsumer();

    const auto metadataTimeoutMs = static_cast<int>(config_.bootstrapTimeout.count() * 1000);

    const auto partitions = discoverPartitions(*consumer_, config_.topic, metadataTimeoutMs);

    PartitionOffsetMap targetOffsets;
    PartitionOffsetMap consumedPositions;

    std::vector<RdKafka::TopicPartition*> assignments;
    assignments.reserve(partitions.size());

    for (const auto partition : partitions) {
        std::int64_t lowWatermark = 0;
        std::int64_t highWatermark = 0;

        const auto error = consumer_->query_watermark_offsets(
            config_.topic, partition, &lowWatermark, &highWatermark, metadataTimeoutMs);

        if (error != RdKafka::ERR_NO_ERROR) {
            destroyTopicPartitions(assignments);

            throw std::runtime_error{
                "Failed to query watermarks for "
                "grid-transfer partition " +
                std::to_string(partition) + ": " + RdKafka::err2str(error)};
        }

        targetOffsets.emplace(partition, highWatermark);

        consumedPositions.emplace(partition, lowWatermark);

        assignments.push_back(RdKafka::TopicPartition::create(config_.topic, partition,
                                                              RdKafka::Topic::OFFSET_BEGINNING));

        spdlog::info(
            "Grid-transfer bootstrap partition {} "
            "range [{}, {})",
            partition, lowWatermark, highWatermark);
    }

    const auto assignmentError = consumer_->assign(assignments);

    destroyTopicPartitions(assignments);

    if (assignmentError != RdKafka::ERR_NO_ERROR) {
        throw std::runtime_error{
            "Failed to assign grid-transfer topic "
            "partitions: " +
            RdKafka::err2str(assignmentError)};
    }

    const auto deadline = std::chrono::steady_clock::now() + config_.bootstrapTimeout;

    while (!allBootstrapTargetsReached(targetOffsets, consumedPositions)) {
        if (std::chrono::steady_clock::now() >= deadline) {
            throw std::runtime_error{
                "Timed out while bootstrapping "
                "GridTransferCache"};
        }

        std::unique_ptr<RdKafka::Message> message{
            consumer_->consume(static_cast<int>(config_.pollTimeout.count()))};

        if (!message) {
            throw std::runtime_error{
                "librdkafka returned a null message "
                "during bootstrap"};
        }

        switch (message->err()) {
            case RdKafka::ERR_NO_ERROR:
                processMessage(*message);

                consumedPositions[message->partition()] =
                    std::max(consumedPositions[message->partition()], message->offset() + 1);
                break;

            case RdKafka::ERR__PARTITION_EOF:
                consumedPositions[message->partition()] =
                    std::max(consumedPositions[message->partition()], message->offset());
                break;

            case RdKafka::ERR__TIMED_OUT:
                break;

            default:
                throw std::runtime_error{
                    "Grid-transfer bootstrap consume "
                    "error: " +
                    message->errstr()};
        }
    }

    cache_.markReady();

    spdlog::info(
        "GridTransferCache bootstrap complete with "
        "{} cached rules",
        cache_.size());
}

void KafkaGridTransferConsumer::startRuntimeUpdates() {
    if (!cache_.isReady()) {
        throw std::logic_error{
            "Cannot consume runtime grid-transfer "
            "updates before bootstrap"};
    }

    if (!consumer_) {
        throw std::logic_error{
            "Kafka grid-transfer consumer is not "
            "initialized"};
    }

    bool expected = false;

    if (!running_.compare_exchange_strong(expected, true, std::memory_order_acq_rel)) {
        return;
    }

    worker_ = std::thread{&KafkaGridTransferConsumer::runtimeLoop, this};
}

void KafkaGridTransferConsumer::runtimeLoop() {
    spdlog::info(
        "Started runtime updates for grid-transfer "
        "topic '{}'",
        config_.topic);

    while (running_.load(std::memory_order_acquire)) {
        std::unique_ptr<RdKafka::Message> message{
            consumer_->consume(static_cast<int>(config_.pollTimeout.count()))};

        if (!message) {
            spdlog::error(
                "librdkafka returned a null "
            );
            continue;
        }

        switch (message->err()) {
            case RdKafka::ERR_NO_ERROR:
                processMessage(*message);
                break;

            case RdKafka::ERR__TIMED_OUT:
            case RdKafka::ERR__PARTITION_EOF:
                break;

            default:
                spdlog::error(
                    "Grid-transfer runtime consume "
                    "error: {}",
                    message->errstr());
                break;
        }
    }

    spdlog::info("Stopped grid-transfer runtime update loop");
}

void KafkaGridTransferConsumer::processMessage(const RdKafka::Message& message) {
    if (message.key() == nullptr) {
        spdlog::warn(
            "Ignoring grid-transfer record without "
            "a Kafka key at partition {} offset {}",
            message.partition(), message.offset());
        return;
    }

    const std::string_view kafkaKey{*message.key()};

    const auto parsedKey = parseGridTransferKey(kafkaKey);

    if (!parsedKey.has_value()) {
        spdlog::warn(
            "Ignoring grid-transfer record with "
            "malformed key '{}' at partition {} "
            "offset {}",
            kafkaKey, message.partition(), message.offset());
        return;
    }

    const auto sellerGridZone = parsedKey->sellerGridZone;

    const auto buyerGridZone = parsedKey->buyerGridZone;

    if (sellerGridZone == buyerGridZone) {
        spdlog::warn(
            "Ignoring same-zone grid-transfer record "
            "'{}'; same-zone transfers are locally "
            "defined as zero-fee",
            kafkaKey);
        return;
    }

    if (message.payload() == nullptr) {
        const bool removed = cache_.remove(sellerGridZone, buyerGridZone);

        spdlog::info(
            "Processed grid-transfer tombstone for "
            "'{}' (removed={})",
            kafkaKey, removed);
        return;
    }

    const auto* payloadBytes = static_cast<const std::byte*>(message.payload());

    const auto protobufRule = ProtobufCodec::deserialize<gridx::grid::v1::GridTransferRule>(
        std::span<const std::byte>{payloadBytes, message.len()});

    if (!protobufRule.has_value()) {
        spdlog::warn(
            "Failed to decode grid-transfer "
            "Protobuf for key '{}' at partition {} "
            "offset {}",
            kafkaKey, message.partition(), message.offset());
        return;
    }

    const auto domainRule = GridTransferEventMapper::toDomain(*protobufRule);

    if (!domainRule.has_value()) {
        spdlog::warn(
            "Grid-transfer rule validation failed "
            "for key '{}' at partition {} offset {}",
            kafkaKey, message.partition(), message.offset());
        return;
    }

    if (domainRule->sellerGridZone != sellerGridZone ||
        domainRule->buyerGridZone != buyerGridZone) {
        spdlog::warn(
            "Grid-transfer Kafka key '{}' does not "
            "match the seller/buyer zones in its "
            "Protobuf value",
            kafkaKey);
        return;
    }

    const bool updated = cache_.upsert(*domainRule);

    spdlog::debug(
        "Processed grid-transfer rule '{}' version "
        "{} (allowed={}, fee={}, updated={})",
        kafkaKey, domainRule->version, domainRule->allowed, domainRule->gridFeePerKwh, updated);
}

void KafkaGridTransferConsumer::stop() noexcept {
    running_.store(false, std::memory_order_release);

    if (worker_.joinable()) {
        worker_.join();
    }

    if (consumer_) {
        const auto error = consumer_->close();

        if (error != RdKafka::ERR_NO_ERROR) {
            spdlog::error(
                "Failed to close grid-transfer "
                "Kafka consumer: {}",
                RdKafka::err2str(error));
        }

        consumer_.reset();
    }

    RdKafka::wait_destroyed(5'000);
}

}  // namespace gridx::matching::adapters::kafka