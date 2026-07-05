#pragma once

#include <string>

namespace gridx::matching::common {

class Version {
public:
    [[nodiscard]] static std::string getVersion();
};

}  // namespace gridx::matching::common