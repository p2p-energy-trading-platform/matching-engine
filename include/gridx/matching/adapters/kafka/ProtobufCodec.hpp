#pragma once

#include <cstddef>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <vector>
#include <cstring>

namespace gridx::matching::adapters::kafka {

class ProtobufCodec {
public:
    ProtobufCodec() = delete;

    /// Generic deserializer for any Protobuf message type T
    template <typename T>
    [[nodiscard]]
    static std::optional<T> deserialize(const std::span<const std::byte> payload) {
        if (payload.empty()) {
            // spdlog::error("Cannot deserialize an empty Kafka payload");
            return std::nullopt;
        }

        if (payload.size() > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
            throw std::length_error("Kafka payload exceeds Protobuf size limit");
        }

        T event;
        const bool parsed = event.ParseFromArray(payload.data(), static_cast<int>(payload.size()));

        if (!parsed) {
            // spdlog::error("Failed to deserialize Protobuf payload into expected message");
            return std::nullopt;
        }

        return event;
    }

    /// Generic serializer for any Protobuf message type T
    template <typename T>
    [[nodiscard]]
    static std::vector<std::byte> serialize(const T& message) {
        const auto size = message.ByteSizeLong();

        if (size <= 0) {
            return {};
        }

        if (size > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
            throw std::length_error("Protobuf message exceeds serialization size limit");
        }

        std::vector<std::byte> payload(size);

        const bool serialized = message.SerializeToArray(payload.data(), static_cast<int>(size));

        if (!serialized) {
            throw std::runtime_error("Failed to serialize Protobuf message");
        }

        return payload;
    }
};

}  // namespace gridx::matching::adapters::kafka