#include "gridx/matching/config/EnvConfigLoader.hpp"

#include <spdlog/spdlog.h>
#include <algorithm>
#include <cctype>
#include <charconv>
#include <chrono>
#include <cstdlib>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>
#include <unordered_map>
#include <vector>

namespace gridx::matching::config {

namespace {

constexpr std::string_view kBootstrapServers = "KAFKA_BOOTSTRAP_SERVERS";

constexpr std::string_view kOrderConsumerGroupId = "KAFKA_ORDER_CONSUMER_GROUP_ID";

constexpr std::string_view kGridTransferConsumerGroupId = "KAFKA_GRID_TRANSFER_CONSUMER_GROUP_ID";

constexpr std::string_view kOrderTopic = "KAFKA_ORDER_TOPIC";

constexpr std::string_view kGridTransferTopic = "KAFKA_GRID_TRANSFER_TOPIC";

constexpr std::string_view kTradeTopic = "KAFKA_TRADE_TOPIC";

constexpr std::string_view kOrderStateTopic = "KAFKA_ORDER_STATE_TOPIC";

constexpr std::string_view kClientId = "KAFKA_CLIENT_ID";

constexpr std::string_view kPollTimeout = "KAFKA_POLL_TIMEOUT_MS";

constexpr std::string_view kBootstrapTimeout = "KAFKA_BOOTSTRAP_TIMEOUT_SECONDS";

constexpr std::string_view kLogLevel = "LOG_LEVEL";

constexpr std::array<std::string_view, 11> kEnvironmentVariables{kBootstrapServers,
                                                                 kOrderConsumerGroupId,
                                                                 kGridTransferConsumerGroupId,
                                                                 kOrderTopic,
                                                                 kGridTransferTopic,
                                                                 kTradeTopic,
                                                                 kOrderStateTopic,
                                                                 kClientId,
                                                                 kPollTimeout,
                                                                 kBootstrapTimeout,

                                                                 kLogLevel};

}  // namespace

namespace {

/**
 * Removes leading and trailing whitespace.
 */
std::string trim(std::string_view value) {
    const auto begin = std::find_if_not(value.begin(), value.end(),
                                        [](unsigned char ch) { return std::isspace(ch); });

    const auto end = std::find_if_not(value.rbegin(), value.rend(), [](unsigned char ch) {
                         return std::isspace(ch);
                     }).base();

    if (begin >= end) {
        return {};
    }

    return std::string(begin, end);
}

/**
 * Converts a string to lowercase.
 */
std::string toLower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });

    return value;
}

/**
 * Parses a positive integer using std::from_chars.
 */
template <typename Integer>
std::optional<Integer> parseInteger(std::string_view value) {
    Integer result{};

    const auto* first = value.data();
    const auto* last = first + value.size();

    const auto [ptr, ec] = std::from_chars(first, last, result);

    if (ec != std::errc{}) {
        return std::nullopt;
    }

    if (ptr != last) {
        return std::nullopt;
    }

    if (result <= 0) {
        return std::nullopt;
    }

    return result;
}

/**
 * Parses the configured log level.
 */
std::optional<spdlog::level::level_enum> parseLogLevel(std::string_view value) {
    const std::string level = toLower(trim(value));

    using Level = spdlog::level::level_enum;

    if (level == "trace") {
        return Level::trace;
    }

    if (level == "debug") {
        return Level::debug;
    }

    if (level == "info") {
        return Level::info;
    }

    if (level == "warn") {
        return Level::warn;
    }

    if (level == "error") {
        return Level::err;
    }

    if (level == "critical") {
        return Level::critical;
    }

    if (level == "off") {
        return Level::off;
    }

    return std::nullopt;
}

/**
 * Reads a required environment variable.
 */
std::string readRequired(const std::unordered_map<std::string, std::string>& environment,
                         std::string_view variableName, std::vector<ValidationError>& errors) {
    const auto it = environment.find(std::string(variableName));

    if (it == environment.end()) {
        errors.push_back(
            {.variableName = std::string(variableName), .reason = "Variable is not set"});

        return {};
    }

    const std::string value = trim(it->second);

    if (value.empty()) {
        errors.push_back(
            {.variableName = std::string(variableName), .reason = "Value must not be empty"});

        return {};
    }

    return value;
}

/**
 * Reads an optional environment variable.
 */
std::string readOptional(const std::unordered_map<std::string, std::string>& environment,
                         std::string_view variableName, std::string_view defaultValue) {
    const auto it = environment.find(std::string(variableName));

    if (it == environment.end()) {
        return std::string(defaultValue);
    }

    const std::string value = trim(it->second);

    if (value.empty()) {
        return std::string(defaultValue);
    }

    return value;
}

/**
 * Reads a required positive integer.
 */
template <typename Integer>
Integer readRequiredInteger(const std::unordered_map<std::string, std::string>& environment,
                            std::string_view variableName, std::vector<ValidationError>& errors) {
    const std::string value = readRequired(environment, variableName, errors);

    if (value.empty()) {
        return {};
    }

    const auto parsed = parseInteger<Integer>(value);

    if (!parsed) {
        errors.push_back({.variableName = std::string(variableName),
                          .reason = "Value must be a positive integer"});

        return {};
    }

    return *parsed;
}

/**
 * Reads an optional positive integer.
 */
template <typename Integer>
Integer readOptionalInteger(const std::unordered_map<std::string, std::string>& environment,
                            std::string_view variableName, std::string_view defaultValue,
                            std::vector<ValidationError>& errors) {
    const std::string value = readOptional(environment, variableName, defaultValue);

    const auto parsed = parseInteger<Integer>(value);

    if (!parsed) {
        errors.push_back({.variableName = std::string(variableName),
                          .reason = "Value must be a positive integer"});

        return {};
    }

    return *parsed;
}

/**
 * Reads the configured log level.
 */
spdlog::level::level_enum readLogLevel(
    const std::unordered_map<std::string, std::string>& environment,
    std::vector<ValidationError>& errors) {
    const std::string value = readOptional(environment, kLogLevel, "info");

    const auto level = parseLogLevel(value);

    if (!level) {
        errors.push_back({.variableName = "LOG_LEVEL", .reason = "Unsupported log level"});

        return spdlog::level::info;
    }

    return *level;
}

}  // namespace

ConfigLoadResult EnvConfigLoader::load(
    const std::unordered_map<std::string, std::string>& environment) {
    ConfigLoadResult result;

    // Kafka settings

    result.config.kafka.bootstrapServers =
        readRequired(environment, kBootstrapServers, result.errors);

    result.config.kafka.clientId = readOptional(environment, kClientId, "matching-engine");

    result.config.kafka.pollTimeout = std::chrono::milliseconds{
        readOptionalInteger<int>(environment, kPollTimeout, "100", result.errors)};

    result.config.kafka.bootstrapTimeout = std::chrono::seconds{
        readOptionalInteger<int>(environment, kBootstrapTimeout, "30", result.errors)};

    // Order consumer

    result.config.orderConsumer.topic = readRequired(environment, kOrderTopic, result.errors);

    result.config.orderConsumer.groupId =
        readRequired(environment, kOrderConsumerGroupId, result.errors);

    // Grid transfer consumer

    result.config.gridTransferConsumer.topic =
        readRequired(environment, kGridTransferTopic, result.errors);

    result.config.gridTransferConsumer.groupId =
        readRequired(environment, kGridTransferConsumerGroupId, result.errors);

    // Publishers
    result.config.publisher.tradeTopic = readRequired(environment, kTradeTopic, result.errors);

    result.config.publisher.orderStateTopic =
        readRequired(environment, kOrderStateTopic, result.errors);

    // Logging
    result.config.logging.level = readLogLevel(environment, result.errors);

    return result;
}

ConfigLoadResult EnvConfigLoader::loadFromEnvironment() {
    std::unordered_map<std::string, std::string> environment;

    for (const auto variable : kEnvironmentVariables) {
        if (const char* value = std::getenv(variable.data()); value != nullptr) {
            environment.emplace(variable, value);
        }
    }

    return load(environment);
}

}  // namespace gridx::matching::config
