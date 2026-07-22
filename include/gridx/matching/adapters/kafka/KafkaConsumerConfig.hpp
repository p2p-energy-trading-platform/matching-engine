#pragma once

#include <string>

namespace gridx::matching::adapters::kafka {

struct KafkaConsumerConfig {
    std::string bootstrapServers;
    std::string consumerGroup;
    std::string orderTopic;
    int pollTimeoutMs{100};
};

}  // namespace gridx::matching::adapters::kafka