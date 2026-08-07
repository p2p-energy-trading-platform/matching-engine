#include "gridx/matching/config/EnvConfigLoader.hpp"

#include <algorithm>
#include <charconv>
#include <cctype>
#include <cstdlib>
#include <optional>
#include <string>
#include <string_view>

namespace gridx::matching::config {

namespace {

/**
 * Removes leading and trailing whitespace.
 */
std::string trim(std::string_view value) {
    const auto begin =
        std::find_if_not(value.begin(), value.end(), [](unsigned char ch) {
            return std::isspace(ch);
        });

    const auto end =
        std::find_if_not(value.rbegin(), value.rend(), [](unsigned char ch) {
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
                   [](unsigned char ch) {
                       return static_cast<char>(std::tolower(ch));
                   });

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
std::optional<spdlog::level::level_enum> parseLogLevel(
    std::string_view value) {

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
std::string readRequired(
    const std::unordered_map<std::string, std::string>& environment,
    std::string_view variableName,
    std::vector<ValidationError>& errors) {

    const auto it = environment.find(std::string(variableName));

    if (it == environment.end()) {
        errors.push_back(
            {.variableName = std::string(variableName),
             .reason = "Variable is not set"});

        return {};
    }

    const std::string value = trim(it->second);

    if (value.empty()) {
        errors.push_back(
            {.variableName = std::string(variableName),
             .reason = "Value must not be empty"});

        return {};
    }

    return value;
}

/**
 * Reads an optional environment variable.
 */
std::string readOptional(
    const std::unordered_map<std::string, std::string>& environment,
    std::string_view variableName,
    std::string_view defaultValue) {

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
Integer readRequiredInteger(
    const std::unordered_map<std::string, std::string>& environment,
    std::string_view variableName,
    std::vector<ValidationError>& errors) {

    const std::string value =
        readRequired(environment, variableName, errors);

    if (value.empty()) {
        return {};
    }

    const auto parsed = parseInteger<Integer>(value);

    if (!parsed) {
        errors.push_back(
            {.variableName = std::string(variableName),
             .reason = "Value must be a positive integer"});

        return {};
    }

    return *parsed;
}

/**
 * Reads an optional positive integer.
 */
template <typename Integer>
Integer readOptionalInteger(
    const std::unordered_map<std::string, std::string>& environment,
    std::string_view variableName,
    std::string_view defaultValue,
    std::vector<ValidationError>& errors) {

    const std::string value =
        readOptional(environment, variableName, defaultValue);

    const auto parsed = parseInteger<Integer>(value);

    if (!parsed) {
        errors.push_back(
            {.variableName = std::string(variableName),
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

    const std::string value =
        readOptional(environment, "LOG_LEVEL", "info");

    const auto level = parseLogLevel(value);

    if (!level) {
        errors.push_back(
            {.variableName = "LOG_LEVEL",
             .reason = "Unsupported log level"});

        return spdlog::level::info;
    }

    return *level;
}

}}