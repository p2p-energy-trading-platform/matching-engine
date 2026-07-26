#pragma once

#include <chrono>
#include <string>

namespace gridx::matching::adapters::kafka {

// NOTE: Need to read env variables instead of hardcoding
struct GridTransferConsumerConfig {
    std::string bootstrapServers;
    std::string groupId;
    std::string topic{"grid.transfer-rules.v1"};

    std::chrono::milliseconds pollTimeout{100};
    std::chrono::seconds bootstrapTimeout{30};
};

}  // namespace gridx::matching::adapters::kafka