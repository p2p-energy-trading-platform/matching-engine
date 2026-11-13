#pragma once

#include "gridx/matching/adapters/kafka/KafkaProducerConfig.hpp"

#include <memory>
#include <span>
#include <string_view>

namespace RdKafka {
class Producer;
}

namespace gridx::matching::adapters::kafka {

class KafkaProducer {
public:
    explicit KafkaProducer(KafkaProducerConfig config);

    ~KafkaProducer();

    // Constructor & Operator overloading:

    // Removes copy constructor
    KafkaProducer(const KafkaProducer&) = delete;

    // Removes copy assignment
    KafkaProducer& operator=(const KafkaProducer&) = delete;

    // Removes move constructor
    KafkaProducer(KafkaProducer&&) = delete;

    // Removes move assignment
    KafkaProducer& operator=(KafkaProducer&&) = delete;

    // Sends a message to the specified Kafka topic with an optional key.
    void send(std::string_view topic, std::span<const std::byte> payload,
              std::string_view key = {});

    void flush();

private:
    void initializeProducer();

    KafkaProducerConfig config_;

    std::unique_ptr<RdKafka::Producer> producer_;
};

}  // namespace gridx::matching::adapters::kafka