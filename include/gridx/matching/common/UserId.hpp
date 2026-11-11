#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <ostream>
#include <string>
#include <string_view>

namespace gridx::matching {


class UserId {
public:
    static constexpr std::size_t kByteCount = 16;
    using Bytes = std::array<std::uint8_t, kByteCount>;

    constexpr UserId() = default;

    explicit constexpr UserId(const Bytes& bytes) : m_bytes(bytes) {}

    
    [[nodiscard]] static constexpr std::optional<UserId> fromString(std::string_view text) {
        if (text.size() != kTextLength) {
            return std::nullopt;
        }

        Bytes bytes{};
        std::size_t byteIndex = 0;

        for (std::size_t i = 0; i < text.size();) {
            if (isDashPosition(i)) {
                if (text[i] != '-') {
                    return std::nullopt;
                }
                ++i;
                continue;
            }

            const int high = hexValue(text[i]);
            const int low = hexValue(text[i + 1]);

            if (high < 0 || low < 0) {
                return std::nullopt;
            }

            bytes[byteIndex++] = static_cast<std::uint8_t>((high << 4) | low);
            i += 2;
        }

        return UserId{bytes};
    }

    // Formats as the canonical lower-case 36-character form.
    [[nodiscard]] std::string toString() const {
        constexpr std::string_view kHexDigits = "0123456789abcdef";

        std::string text;
        text.reserve(kTextLength);

        for (std::size_t i = 0; i < kByteCount; ++i) {
            if (i == 4 || i == 6 || i == 8 || i == 10) {
                text.push_back('-');
            }
            text.push_back(kHexDigits[m_bytes[i] >> 4]);
            text.push_back(kHexDigits[m_bytes[i] & 0x0F]);
        }

        return text;
    }

    [[nodiscard]] constexpr bool isNil() const {
        for (const auto byte : m_bytes) {
            if (byte != 0) {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] constexpr const Bytes& bytes() const {
        return m_bytes;
    }

    friend constexpr bool operator==(const UserId&, const UserId&) = default;

    friend std::ostream& operator<<(std::ostream& os, const UserId& userId) {
        return os << userId.toString();
    }

private:
    static constexpr std::size_t kTextLength = 36;

    static constexpr bool isDashPosition(std::size_t index) {
        return index == 8 || index == 13 || index == 18 || index == 23;
    }

    static constexpr int hexValue(char c) {
        if (c >= '0' && c <= '9') {
            return c - '0';
        }
        if (c >= 'a' && c <= 'f') {
            return c - 'a' + 10;
        }
        if (c >= 'A' && c <= 'F') {
            return c - 'A' + 10;
        }
        return -1;
    }

    Bytes m_bytes{};
};

}  // namespace gridx::matching
