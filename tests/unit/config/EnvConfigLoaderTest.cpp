#include <gtest/gtest.h>
#include <algorithm>
#include <array>
#include <ranges>
#include <string_view>

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

};

TEST_F(EnvConfigLoaderTest, LoadsValidConfiguration) {
    const Environment environment = createValidEnvironment();

    const ConfigLoadResult result = EnvConfigLoader::load(environment);

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

    const ConfigLoadResult result = EnvConfigLoader::load(environment);

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

    const ConfigLoadResult result = EnvConfigLoader::load(environment);

    EXPECT_FALSE(result.success());
    ASSERT_EQ(result.errors.size(), 1U);

    EXPECT_EQ(result.errors.front().variableName,
              "KAFKA_BOOTSTRAP_SERVERS");
}

TEST_F(EnvConfigLoaderTest, ReportsMissingOrderConsumerGroupId) {
    Environment environment = createValidEnvironment();

    environment.erase("KAFKA_ORDER_CONSUMER_GROUP_ID");

    const ConfigLoadResult result = EnvConfigLoader::load(environment);

    EXPECT_FALSE(result.success());
    ASSERT_EQ(result.errors.size(), 1U);

    EXPECT_EQ(result.errors.front().variableName,
              "KAFKA_ORDER_CONSUMER_GROUP_ID");
}

TEST_F(EnvConfigLoaderTest, ReportsMissingGridTransferConsumerGroupId) {
    Environment environment = createValidEnvironment();

    environment.erase("KAFKA_GRID_TRANSFER_CONSUMER_GROUP_ID");

    const ConfigLoadResult result = EnvConfigLoader::load(environment);

    EXPECT_FALSE(result.success());
    ASSERT_EQ(result.errors.size(), 1U);

    EXPECT_EQ(result.errors.front().variableName,
              "KAFKA_GRID_TRANSFER_CONSUMER_GROUP_ID");
}

TEST_F(EnvConfigLoaderTest, ReportsMissingOrderTopic) {
    Environment environment = createValidEnvironment();

    environment.erase("KAFKA_ORDER_TOPIC");

    const ConfigLoadResult result = EnvConfigLoader::load(environment);

    EXPECT_FALSE(result.success());
    ASSERT_EQ(result.errors.size(), 1U);

    EXPECT_EQ(result.errors.front().variableName,
              "KAFKA_ORDER_TOPIC");
}

TEST_F(EnvConfigLoaderTest, ReportsMissingGridTransferTopic) {
    Environment environment = createValidEnvironment();

    environment.erase("KAFKA_GRID_TRANSFER_TOPIC");

    const ConfigLoadResult result = EnvConfigLoader::load(environment);

    EXPECT_FALSE(result.success());
    ASSERT_EQ(result.errors.size(), 1U);

    EXPECT_EQ(result.errors.front().variableName,
              "KAFKA_GRID_TRANSFER_TOPIC");
}

TEST_F(EnvConfigLoaderTest, ReportsMissingTradeTopic) {
    Environment environment = createValidEnvironment();

    environment.erase("KAFKA_TRADE_TOPIC");

    const ConfigLoadResult result = EnvConfigLoader::load(environment);

    EXPECT_FALSE(result.success());
    ASSERT_EQ(result.errors.size(), 1U);

    EXPECT_EQ(result.errors.front().variableName,
              "KAFKA_TRADE_TOPIC");
}

TEST_F(EnvConfigLoaderTest, ReportsMissingOrderStateTopic) {
    Environment environment = createValidEnvironment();

    environment.erase("KAFKA_ORDER_STATE_TOPIC");

    const ConfigLoadResult result = EnvConfigLoader::load(environment);

    EXPECT_FALSE(result.success());
    ASSERT_EQ(result.errors.size(), 1U);

    EXPECT_EQ(result.errors.front().variableName,
              "KAFKA_ORDER_STATE_TOPIC");
}

TEST_F(EnvConfigLoaderTest, RejectsEmptyRequiredValue) {
    Environment environment = createValidEnvironment();

    environment["KAFKA_ORDER_TOPIC"] = "";

    const ConfigLoadResult result = EnvConfigLoader::load(environment);

    EXPECT_FALSE(result.success());
    ASSERT_EQ(result.errors.size(), 1U);

    EXPECT_EQ(result.errors.front().variableName,
              "KAFKA_ORDER_TOPIC");
}

TEST_F(EnvConfigLoaderTest, RejectsWhitespaceOnlyRequiredValue) {
    Environment environment = createValidEnvironment();

    environment["KAFKA_ORDER_TOPIC"] = "      ";

    const ConfigLoadResult result = EnvConfigLoader::load(environment);

    EXPECT_FALSE(result.success());
    ASSERT_EQ(result.errors.size(), 1U);

    EXPECT_EQ(result.errors.front().variableName,
              "KAFKA_ORDER_TOPIC");
}

TEST_F(EnvConfigLoaderTest, RejectsNonNumericPollTimeout) {
    Environment environment = createValidEnvironment();

    environment["KAFKA_POLL_TIMEOUT_MS"] = "abc";

    const ConfigLoadResult result = EnvConfigLoader::load(environment);

    EXPECT_FALSE(result.success());
    ASSERT_EQ(result.errors.size(), 1U);

    EXPECT_EQ(result.errors.front().variableName,
              "KAFKA_POLL_TIMEOUT_MS");
}

TEST_F(EnvConfigLoaderTest, RejectsNegativePollTimeout) {
    Environment environment = createValidEnvironment();

    environment["KAFKA_POLL_TIMEOUT_MS"] = "-100";

    const ConfigLoadResult result = EnvConfigLoader::load(environment);

    EXPECT_FALSE(result.success());
    ASSERT_EQ(result.errors.size(), 1U);

    EXPECT_EQ(result.errors.front().variableName,
              "KAFKA_POLL_TIMEOUT_MS");
}

TEST_F(EnvConfigLoaderTest, RejectsZeroPollTimeout) {
    Environment environment = createValidEnvironment();

    environment["KAFKA_POLL_TIMEOUT_MS"] = "0";

    const ConfigLoadResult result = EnvConfigLoader::load(environment);

    EXPECT_FALSE(result.success());
    ASSERT_EQ(result.errors.size(), 1U);

    EXPECT_EQ(result.errors.front().variableName,
              "KAFKA_POLL_TIMEOUT_MS");
}

TEST_F(EnvConfigLoaderTest, RejectsTrailingCharactersInPollTimeout) {
    Environment environment = createValidEnvironment();

    environment["KAFKA_POLL_TIMEOUT_MS"] = "100ms";

    const ConfigLoadResult result = EnvConfigLoader::load(environment);

    EXPECT_FALSE(result.success());
    ASSERT_EQ(result.errors.size(), 1U);

    EXPECT_EQ(result.errors.front().variableName,
              "KAFKA_POLL_TIMEOUT_MS");
}

TEST_F(EnvConfigLoaderTest, RejectsInvalidBootstrapTimeout) {
    Environment environment = createValidEnvironment();

    environment["KAFKA_BOOTSTRAP_TIMEOUT_SECONDS"] = "30seconds";

    const ConfigLoadResult result = EnvConfigLoader::load(environment);

    EXPECT_FALSE(result.success());
    ASSERT_EQ(result.errors.size(), 1U);

    EXPECT_EQ(result.errors.front().variableName,
              "KAFKA_BOOTSTRAP_TIMEOUT_SECONDS");
}

TEST_F(EnvConfigLoaderTest, AcceptsEverySupportedLogLevel) {
    constexpr std::array<std::pair<std::string_view, spdlog::level::level_enum>, 7> testCases{{
        {"trace", spdlog::level::trace},
        {"debug", spdlog::level::debug},
        {"info", spdlog::level::info},
        {"warn", spdlog::level::warn},
        {"error", spdlog::level::err},
        {"critical", spdlog::level::critical},
        {"off", spdlog::level::off},
    }};

    for (const auto& [levelName, expectedLevel] : testCases) {
        Environment environment = createValidEnvironment();

        environment["LOG_LEVEL"] = std::string(levelName);

        const ConfigLoadResult result = EnvConfigLoader::load(environment);

        ASSERT_TRUE(result.success());

        EXPECT_EQ(result.config.logging.level, expectedLevel);
    }
}

TEST_F(EnvConfigLoaderTest, AcceptsCaseInsensitiveLogLevels) {
    Environment environment = createValidEnvironment();

    environment["LOG_LEVEL"] = "DeBuG";

    const ConfigLoadResult result = EnvConfigLoader::load(environment);

    ASSERT_TRUE(result.success());

    EXPECT_EQ(result.config.logging.level,
              spdlog::level::debug);
}

TEST_F(EnvConfigLoaderTest, RejectsUnsupportedLogLevel) {
    Environment environment = createValidEnvironment();

    environment["LOG_LEVEL"] = "verbose";

    const ConfigLoadResult result = EnvConfigLoader::load(environment);

    EXPECT_FALSE(result.success());
    ASSERT_EQ(result.errors.size(), 1U);

    EXPECT_EQ(result.errors.front().variableName,
              "LOG_LEVEL");
}

TEST_F(EnvConfigLoaderTest, ReportsMultipleValidationErrors) {
    Environment environment = createValidEnvironment();

    environment.erase("KAFKA_BOOTSTRAP_SERVERS");
    environment.erase("KAFKA_ORDER_TOPIC");

    environment["KAFKA_POLL_TIMEOUT_MS"] = "100ms";
    environment["LOG_LEVEL"] = "verbose";

    const ConfigLoadResult result = EnvConfigLoader::load(environment);

    EXPECT_FALSE(result.success());

    EXPECT_EQ(result.errors.size(), 4U);

    EXPECT_TRUE(std::ranges::any_of(
        result.errors,
        [](const ValidationError& error) {
            return error.variableName == "KAFKA_BOOTSTRAP_SERVERS";
        }));

    EXPECT_TRUE(std::ranges::any_of(
        result.errors,
        [](const ValidationError& error) {
            return error.variableName == "KAFKA_ORDER_TOPIC";
        }));

    EXPECT_TRUE(std::ranges::any_of(
        result.errors,
        [](const ValidationError& error) {
            return error.variableName == "KAFKA_POLL_TIMEOUT_MS";
        }));

    EXPECT_TRUE(std::ranges::any_of(
        result.errors,
        [](const ValidationError& error) {
            return error.variableName == "LOG_LEVEL";
        }));
}

TEST_F(EnvConfigLoaderTest, TrimsWhitespaceFromValues) {
    Environment environment = createValidEnvironment();

    environment["KAFKA_BOOTSTRAP_SERVERS"] = "   localhost:9092   ";
    environment["KAFKA_CLIENT_ID"] = "   matching-engine   ";

    const ConfigLoadResult result = EnvConfigLoader::load(environment);

    ASSERT_TRUE(result.success());

    EXPECT_EQ(result.config.kafka.bootstrapServers,
              "localhost:9092");

    EXPECT_EQ(result.config.kafka.clientId,
              "matching-engine");
}

TEST_F(EnvConfigLoaderTest, AppliesDefaultsWhenOptionalValuesAreWhitespace) {
    Environment environment = createValidEnvironment();

    environment["KAFKA_CLIENT_ID"] = "   ";
    environment["KAFKA_POLL_TIMEOUT_MS"] = "   ";
    environment["KAFKA_BOOTSTRAP_TIMEOUT_SECONDS"] = "   ";
    environment["LOG_LEVEL"] = "   ";

    const ConfigLoadResult result = EnvConfigLoader::load(environment);

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

}  // namespace

}  // namespace gridx::matching::config