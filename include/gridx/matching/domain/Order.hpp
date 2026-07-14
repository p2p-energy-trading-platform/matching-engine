#pragma once

#include "gridx/matching/common/Types.hpp"
#include "gridx/matching/domain/MarketId.hpp"

namespace gridx::matching
{

struct Order
{
    OrderId orderId;
    UserId userId;

    MarketId marketId;
    GridZoneId gridZone;

    Side side;
    OrderType orderType;
    OrderStatus status;

    Price price;
    Quantity quantity;
    Quantity remainingQuantity;

    Timestamp deliverySlotStart;
    Timestamp deliverySlotEnd;

    Timestamp expiresAt;
    Timestamp timestamp;
};

} // namespace gridx::matching