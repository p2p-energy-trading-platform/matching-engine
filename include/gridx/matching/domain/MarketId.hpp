#pragma once

#include <chrono>

#include "gridx/matching/common/Types.hpp"

namespace gridx::matching {

constexpr auto kDeliverySlotDuration = std::chrono::minutes{30};

// NOTE:
// MarketId currently identifies a Market Book by its delivery slot.
// It can be extended in the future if additional market dimensions
// (e.g., product type) are introduced.
struct MarketId {
    Timestamp deliverySlotStart;

    [[nodiscard]]
    Timestamp deliverySlotEnd() const {
        return deliverySlotStart + kDeliverySlotDuration;
    }

    bool operator==(const MarketId&) const = default;
};

}  // namespace gridx::matching