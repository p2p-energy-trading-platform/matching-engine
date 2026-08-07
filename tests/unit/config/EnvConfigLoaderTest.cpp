#include <gtest/gtest.h>

#include <unordered_map>

#include "gridx/matching/config/EnvConfigLoader.hpp"

namespace gridx::matching::config {

namespace {

using Environment = std::unordered_map<std::string, std::string>;

Environment createValidEnvironment() {
    return {
        {"KAFKA_BOOTSTRAP_SERVERS", "localhost:9092"},
        {"KAFKA_ORDER_CONSUMER_GROUP_ID", "matching-order"},
        {"KAFKA_GRID_TRANSFER_CONSUMER_GROUP_ID", "matching-grid"},
        {"KAFKA_ORDER_TOPIC", "order.accepted"},
        {"KAFKA_GRID_TRANSFER_TOPIC", "grid.transfer"},
        {"KAFKA_TRADE_TOPIC", "trade.executed"},
        {"KAFKA_ORDER_STATE_TOPIC", "order.state"},
        {"KAFKA_CLIENT_ID", "matching-engine"},
        {"KAFKA_POLL_TIMEOUT_MS", "100"},
        {"KAFKA_BOOTSTRAP_TIMEOUT_SECONDS", "30"},
        {"LOG_LEVEL", "debug"},
    };
}

class EnvConfigLoaderTest : public ::testing::Test {
protected:
    EnvConfigLoader loader;
};

TEST_F(EnvConfigLoaderTest, LoadsValidConfiguration) {
    const Environment environment = createValidEnvironment();

    const ConfigLoadResult result = loader.load(environment);

    ASSERT_TRUE(result.success());
    EXPECT_TRUE(result.errors.empty());

    EXPECT_EQ(result.config.kafka.bootstrapServers, "localhost:9092");
    EXPECT_EQ(result.config.kafka.clientId, "matching-engine");
    EXPECT_EQ(result.config.kafka.pollTimeout,
              std::chrono::milliseconds{100});
    EXPECT_EQ(result.config.kafka.bootstrapTimeout,
              std::chrono::seconds{30});

    EXPECT_EQ(result.config.orderConsumer.topic,
              "order.accepted");
    EXPECT_EQ(result.config.orderConsumer.groupId,
              "matching-order");

    EXPECT_EQ(result.config.gridTransferConsumer.topic,
              "grid.transfer");
    EXPECT_EQ(result.config.gridTransferConsumer.groupId,
              "matching-grid");

    EXPECT_EQ(result.config.publisher.tradeTopic,
              "trade.executed");
    EXPECT_EQ(result.config.publisher.orderStateTopic,
              "order.state");

    EXPECT_EQ(result.config.logging.level,
              spdlog::level::debug);
}

TEST_F(EnvConfigLoaderTest, UsesDefaultValuesForOptionalVariables) {
    Environment environment = createValidEnvironment();

    environment.erase("KAFKA_CLIENT_ID");
    environment.erase("KAFKA_POLL_TIMEOUT_MS");
    environment.erase("KAFKA_BOOTSTRAP_TIMEOUT_SECONDS");
    environment.erase("LOG_LEVEL");

    const ConfigLoadResult result = loader.load(environment);

    ASSERT_TRUE(result.success());

    EXPECT_EQ(result.config.kafka.clientId,
              "matching-engine");

    EXPECT_EQ(result.config.kafka.pollTimeout,
              std::chrono::milliseconds{100});

    EXPECT_EQ(result.config.kafka.bootstrapTimeout,
              std::chrono::seconds{30});

    EXPECT_EQ(result.config.logging.level,
              spdlog::level::info);
}

TEST_F(EnvConfigLoaderTest, ReportsMissingBootstrapServers) {
    Environment environment = createValidEnvironment();

    environment.erase("KAFKA_BOOTSTRAP_SERVERS");

    const ConfigLoadResult result = loader.load(environment);

    EXPECT_FALSE(result.success());
    ASSERT_EQ(result.errors.size(), 1U);

    EXPECT_EQ(result.errors.front().variableName,
              "KAFKA_BOOTSTRAP_SERVERS");
}

TEST_F(EnvConfigLoaderTest, ReportsMissingOrderConsumerGroupId) {
    Environment environment = createValidEnvironment();

    environment.erase("KAFKA_ORDER_CONSUMER_GROUP_ID");

    const ConfigLoadResult result = loader.load(environment);

    EXPECT_FALSE(result.success());
    ASSERT_EQ(result.errors.size(), 1U);

    EXPECT_EQ(result.errors.front().variableName,
              "KAFKA_ORDER_CONSUMER_GROUP_ID");
}

TEST_F(EnvConfigLoaderTest, ReportsMissingGridTransferConsumerGroupId) {
    Environment environment = createValidEnvironment();

    environment.erase("KAFKA_GRID_TRANSFER_CONSUMER_GROUP_ID");

    const ConfigLoadResult result = loader.load(environment);

    EXPECT_FALSE(result.success());
    ASSERT_EQ(result.errors.size(), 1U);

    EXPECT_EQ(result.errors.front().variableName,
              "KAFKA_GRID_TRANSFER_CONSUMER_GROUP_ID");
}

TEST_F(EnvConfigLoaderTest, ReportsMissingOrderTopic) {
    Environment environment = createValidEnvironment();

    environment.erase("KAFKA_ORDER_TOPIC");

    const ConfigLoadResult result = loader.load(environment);

    EXPECT_FALSE(result.success());
    ASSERT_EQ(result.errors.size(), 1U);

    EXPECT_EQ(result.errors.front().variableName,
              "KAFKA_ORDER_TOPIC");
}

TEST_F(EnvConfigLoaderTest, ReportsMissingGridTransferTopic) {
    Environment environment = createValidEnvironment();

    environment.erase("KAFKA_GRID_TRANSFER_TOPIC");

    const ConfigLoadResult result = loader.load(environment);

    EXPECT_FALSE(result.success());
    ASSERT_EQ(result.errors.size(), 1U);

    EXPECT_EQ(result.errors.front().variableName,
              "KAFKA_GRID_TRANSFER_TOPIC");
}

TEST_F(EnvConfigLoaderTest, ReportsMissingTradeTopic) {
    Environment environment = createValidEnvironment();

    environment.erase("KAFKA_TRADE_TOPIC");

    const ConfigLoadResult result = loader.load(environment);

    EXPECT_FALSE(result.success());
    ASSERT_EQ(result.errors.size(), 1U);

    EXPECT_EQ(result.errors.front().variableName,
              "KAFKA_TRADE_TOPIC");
}

TEST_F(EnvConfigLoaderTest, ReportsMissingOrderStateTopic) {
    Environment environment = createValidEnvironment();

    environment.erase("KAFKA_ORDER_STATE_TOPIC");

    const ConfigLoadResult result = loader.load(environment);

    EXPECT_FALSE(result.success());
    ASSERT_EQ(result.errors.size(), 1U);

    EXPECT_EQ(result.errors.front().variableName,
              "KAFKA_ORDER_STATE_TOPIC");
}

TEST_F(EnvConfigLoaderTest, RejectsEmptyRequiredValue) {
    Environment environment = createValidEnvironment();

    environment["KAFKA_ORDER_TOPIC"] = "";

    const ConfigLoadResult result = loader.load(environment);

    EXPECT_FALSE(result.success());
    ASSERT_EQ(result.errors.size(), 1U);

    EXPECT_EQ(result.errors.front().variableName,
              "KAFKA_ORDER_TOPIC");
}

TEST_F(EnvConfigLoaderTest, RejectsWhitespaceOnlyRequiredValue) {
    Environment environment = createValidEnvironment();

    environment["KAFKA_ORDER_TOPIC"] = "      ";

    const ConfigLoadResult result = loader.load(environment);

    EXPECT_FALSE(result.success());
    ASSERT_EQ(result.errors.size(), 1U);

    EXPECT_EQ(result.errors.front().variableName,
              "KAFKA_ORDER_TOPIC");
}


}  // namespace

}  // namespace gridx::matching::config