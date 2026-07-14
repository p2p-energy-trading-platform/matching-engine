#pragma once

#include "gridx/matching/common/Types.hpp"

namespace gridx::matching {
// NOTE:
// MarketId currently identifies a Market Book by its delivery slot
// It can be extended in the future if additional market dimensions
// (e.g., product type) are introduced.
struct MarketId {
    Timestamp deliverySlotStart;
};

}  // namespace gridx::matching