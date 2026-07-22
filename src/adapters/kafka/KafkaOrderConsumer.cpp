#include "gridx/matching/adapters/kafka/KafkaOrderConsumer.hpp"

#include <librdkafka/rdkafkacpp.h>
#include <spdlog/spdlog.h>

#include <cstddef>
#include <memory>
#include <span>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace gridx::matching::adapters::kafka {

namespace {

// The conf comes the the librdkafka pacakge.
// NOTE: There seems to be an error with vscode not identifying includes
// FIX LATER
void setConfig(RdKafka::Conf& config, const std::string& key, const std::string& value) {
    std::string error;

    const auto result = config.set(key, value, error);

    if (result != RdKafka::Conf::CONF_OK) {
        throw std::runtime_error("Failed to set Kafka property '" + key + "': " + error);
    }
}

}  // namespace

KafkaOrderConsumer::KafkaOrderConsumer(KafkaConsumerConfig config,
                                       engine::OrderProcessor& orderProcessor)
    : config_{std::move(config)}, orderProcessor_{orderProcessor} {}

KafkaOrderConsumer::~KafkaOrderConsumer() {
    stop();

    if (consumer_) {
        const auto result = consumer_->close();

        if (result != RdKafka::ERR_NO_ERROR) {
            spdlog::error("Failed to close Kafka consumer: {}", RdKafka::err2str(result));
        }
    }
}

void KafkaOrderConsumer::initializeConsumer() {
    std::unique_ptr<RdKafka::Conf> kafkaConfig{RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL)};

    if (!kafkaConfig) {
        throw std::runtime_error("Failed to create Kafka consumer configuration");
    }

    setConfig(*kafkaConfig, "bootstrap.servers", config_.bootstrapServers);

    setConfig(*kafkaConfig, "group.id", config_.consumerGroup);

    setConfig(*kafkaConfig, "enable.auto.commit", "false");

    setConfig(*kafkaConfig, "auto.offset.reset", "earliest");

    std::string error;

    consumer_.reset(RdKafka::KafkaConsumer::create(kafkaConfig.get(), error));

    if (!consumer_) {
        throw std::runtime_error("Failed to create Kafka consumer: " + error);
    }
}

void KafkaOrderConsumer::start() {
    if (running_.exchange(true)) {
        throw std::logic_error("Kafka order consumer is already running");
    }

    try {
        initializeConsumer();

        const auto result = consumer_->subscribe(std::vector<std::string>{config_.orderTopic});

        if (result != RdKafka::ERR_NO_ERROR) {
            throw std::runtime_error("Failed to subscribe to Kafka topic '" + config_.orderTopic +
                                     "': " + RdKafka::err2str(result));
        }

        spdlog::info("Subscribed to Kafka order topic '{}'", config_.orderTopic);

        consumeLoop();
    } catch (...) {
        running_.store(false);
        throw;
    }
}

void KafkaOrderConsumer::stop() noexcept {
    running_.store(false);
}

void KafkaOrderConsumer::consumeLoop() {
    while (running_.load()) {
        // NOTE: Is polling the right approach? Need to optimize this later
        std::unique_ptr<RdKafka::Message> message{consumer_->consume(config_.pollTimeoutMs)};

        if (!message) {
            spdlog::warn("Kafka consumer returned a null message");
            continue;
        }

        switch (message->err()) {
            case RdKafka::ERR_NO_ERROR: {
                if (message->payload() == nullptr || message->len() == 0) {
                    spdlog::warn("Received an empty Kafka order message");
                    break;
                }

                const auto* payload = static_cast<const std::byte*>(message->payload());

                const std::span<const std::byte> payloadView{payload, message->len()};

                try {
                    orderProcessor_.process(payloadView);

                    const auto commitResult = consumer_->commitSync(message.get());

                    if (commitResult != RdKafka::ERR_NO_ERROR) {
                        spdlog::error("Failed to commit Kafka offset {}: {}", message->offset(),
                                      RdKafka::err2str(commitResult));
                    }
                } catch (const std::exception& exception) {
                    spdlog::error(
                        "Failed to process Kafka order message "
                        "from topic '{}', partition {}, offset {}: {}",
                        message->topic_name(), message->partition(), message->offset(),
                        exception.what());

                    // Offset intentionally not committed.
                }

                break;
            }

            case RdKafka::ERR__TIMED_OUT:
                break;

            case RdKafka::ERR__PARTITION_EOF:
                spdlog::debug("Reached end of topic '{}', partition {}", message->topic_name(),
                              message->partition());
                break;

            default:
                spdlog::error("Kafka consumer error: {}", message->errstr());
                break;
        }
    }
}

}  // namespace gridx::matching::adapters::kafka