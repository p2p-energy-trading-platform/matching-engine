#pragma once

#include <string>

namespace gridx::matching::adapters::kafka {

struct KafkaConsumerConfig {
    std::string bootstrap_servers;
    std::string consumer_group;
    std::string order_topic;
    int poll_timeout_ms{100};
};

} // namespace gridx::matching::adapters::kafka