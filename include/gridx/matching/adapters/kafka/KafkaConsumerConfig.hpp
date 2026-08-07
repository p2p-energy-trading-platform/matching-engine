#pragma once

#include <chrono>
#include <string>

namespace gridx::matching::adapters::kafka {

struct KafkaConsumerConfig {
    std::string bootstrapServers;
    std::string clientId;
    std::string consumerGroup;
    std::string topic;
    std::chrono::milliseconds pollTimeout;
    std::chrono::seconds bootstrapTimeout;
};

}  // namespace gridx::matching::adapters::kafka