#pragma once

#include <chrono>
#include <string>

#include <spdlog/common.h>

namespace gridx::matching::config {

struct KafkaSettings {
    std::string bootstrapServers;
    std::string clientId;

    std::chrono::milliseconds pollTimeout;
    std::chrono::seconds bootstrapTimeout;
};

struct OrderConsumerSettings {
    std::string topic;
    std::string groupId;
};

struct GridTransferConsumerSettings {
    std::string topic;
    std::string groupId;
};

struct PublisherSettings {
    std::string tradeTopic;
    std::string orderStateTopic;
};

struct LoggingSettings {
    spdlog::level::level_enum level;
};

struct AppConfig {
    KafkaSettings kafka;

    OrderConsumerSettings orderConsumer;

    GridTransferConsumerSettings gridTransferConsumer;

    PublisherSettings publisher;

    LoggingSettings logging;
};

}  // namespace gridx::matching::config