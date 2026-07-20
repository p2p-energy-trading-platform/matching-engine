#include "gridx/matching/adapters/kafka/ProtobufOrderCodec.hpp"

namespace gridx::matching::adapters::kafka {

gridx::order::v1::OrderAccepted ProtobufOrderCodec::deserialize(
    const std::span<const std::byte> payload) const {
    if (payload.empty()) {
        throw std::invalid_argument("Cannot deserialize an empty kafka order payload");
    }

    if (payload.size() > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
        throw std::length_error("Kafka order payload exceeds Protobuf size limit");
    }

    gridx::order::v1::OrderAccepted event;

    const bool parsed = event.ParseFromArray(payload.data(), static_cast<int>(payload.size()));

    if (!parsed) {
        throw std::runtime_error("Failed to deserialize OrderAccepted event");
    }

    return event;
}

}  // namespace gridx::matching::adapters::kafka