#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "gridx/matching/config/AppConfig.hpp"

namespace gridx::matching::config {

struct ValidationError {
    std::string variableName;
    std::string reason;
};

struct ConfigLoadResult {
    AppConfig config;

    std::vector<ValidationError> errors;

    [[nodiscard]]
    bool success() const noexcept {
        return errors.empty();
    }
};

class EnvConfigLoader {
public:
    EnvConfigLoader() = delete;

    /**
     * Loads and validates configuration from a supplied environment map.
     * This overload is intended for unit testing.
     */
    [[nodiscard]]
    static ConfigLoadResult load(
        const std::unordered_map<std::string, std::string>& environment);

    /**
     * Loads and validates configuration from the current process environment.
     */
    [[nodiscard]]
    static ConfigLoadResult loadFromEnvironment();
};

}  // namespace gridx::matching::config