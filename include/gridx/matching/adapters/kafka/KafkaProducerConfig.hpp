#pragma once

#include <string>

namespace gridx::matching::adapters::kafka {

struct KafkaProducerConfig {
    std::string bootstrapServers;
    std::string clientId;
    std::string orderTopic;
};

}  // namespace gridx::matching::adapters::kafka