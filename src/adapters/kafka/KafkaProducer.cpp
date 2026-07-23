#include "gridx/matching/adapters/kafka/KafkaProducer.hpp"

#include <librdkafka/rdkafkacpp.h>

#include <stdexcept>
#include <string>

namespace gridx::matching::adapters::kafka {

KafkaProducer::KafkaProducer(KafkaProducerConfig config) : config_(std::move(config)) {
    initializeProducer();
}

KafkaProducer::~KafkaProducer() {
    flush();
}

void KafkaProducer::initializeProducer() {
    std::string error;

    auto configuration =
        std::unique_ptr<RdKafka::Conf>(RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL));

    if (configuration == nullptr) {
        throw std::runtime_error("Failed to create Kafka producer configuration");
    }

    if (configuration->set("bootstrap.servers", config_.bootstrapServers, error) !=
        RdKafka::Conf::CONF_OK) {
        throw std::runtime_error("Failed to configure bootstrap.servers: " + error);
    }

    if (configuration->set("client.id", config_.clientId, error) != RdKafka::Conf::CONF_OK) {
        throw std::runtime_error(error);
    }

    producer_.reset(RdKafka::Producer::create(configuration.get(), error));

    if (producer_ == nullptr) {
        throw std::runtime_error("Failed to create Kafka producer: " + error);
    }
}

void KafkaProducer::send(std::string_view topic, std::span<const std::byte> payload) {
    if (topic.empty()) {
        throw std::invalid_argument("Kafka topic cannot be empty");
    }

    const auto result = producer_->produce(std::string(topic), RdKafka::Topic::PARTITION_UA,
                                           RdKafka::Producer::RK_MSG_COPY,
                                           const_cast<std::byte*>(payload.data()), payload.size(),
                                           nullptr,   
                                           0,         
                                           0,        
                                           nullptr);

    if (result != RdKafka::ERR_NO_ERROR) {
        throw std::runtime_error("Failed to publish Kafka message: " + RdKafka::err2str(result));
    }

    // Serve delivery callbacks.
    producer_->poll(0);
}

void KafkaProducer::flush() {
    if (producer_ != nullptr) {
        producer_->flush(5000);
    }
}

}  // namespace gridx::matching::adapters::kafka