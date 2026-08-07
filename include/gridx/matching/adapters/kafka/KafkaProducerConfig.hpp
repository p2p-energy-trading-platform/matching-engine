#pragma once

#include <chrono>
#include <string>

namespace gridx::matching::adapters::kafka {

struct KafkaProducerConfig {
    std::string bootstrapServers;
    std::string clientId;
    std::chrono::seconds bootstrapTimeout;
};

}  // namespace gridx::matching::adapters::kafka