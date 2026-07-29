#pragma once

#include <chrono>
#include <string>

namespace gridx::matching::adapters::kafka {

struct KafkaConsumerConfig {
    std::string bootstrapServers;
    std::string consumerGroup;
    std::string topic;
    std::chrono::milliseconds pollTimeout{100};
    std::chrono::seconds bootstrapTimeout{30};
};

}  // namespace gridx::matching::adapters::kafka