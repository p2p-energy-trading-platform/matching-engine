#include "gridx/matching/adapters/kafka/OrderEventMapper.hpp"

#include <chrono>

namespace gridx::matching::adapters::kafka {

namespace {

Timestamp toTimestamp(const google::protobuf::Timestamp& timestamp) {
    using namespace std::chrono;

    return system_clock::time_point{seconds{timestamp.seconds()} + nanoseconds{timestamp.nanos()}};
}

Side toDomainSide(const gridx::order::v1::Side side) {
    using ProtoSide = gridx::order::v1::Side;

    switch (side) {
        case ProtoSide::SIDE_BUY:
            return Side::Buy;
        case ProtoSide::SIDE_SELL:
            return Side::Sell;
        case ProtoSide::SIDE_UNSPECIFIED:
        default:
            throw std::invalid_argument("Order event has an invalid side");
    }
}

OrderType toDomainOrderType(const gridx::order::v1::OrderType orderType) {
    using ProtoOrderType = gridx::order::v1::OrderType;

    switch (orderType) {
        case ProtoOrderType::ORDER_TYPE_LIMIT:
            return OrderType::Limit;
        case ProtoOrderType::ORDER_TYPE_UNSPECIFIED:
        default:
            throw std::invalid_argument("Order event has an invalid order type");
    }
}

OrderStatus toDomainOrderStatus(const gridx::order::v1::OrderStatus orderStatus) {
    using ProtoOrderStatus = gridx::order::v1::OrderStatus;

    switch (orderStatus) {
        case ProtoOrderStatus::ORDER_STATUS_NEW:
            return OrderStatus::New;
        case ProtoOrderStatus::ORDER_STATUS_PARTIALLY_FILLED:
            return OrderStatus::PartiallyFilled;
        case ProtoOrderStatus::ORDER_STATUS_FILLED:
            return OrderStatus::Filled;
        case ProtoOrderStatus::ORDER_STATUS_EXPIRED:
            return OrderStatus::Expired;
        case ProtoOrderStatus::ORDER_STATUS_CANCELLED:
            return OrderStatus::Cancelled;
        case ProtoOrderStatus::ORDER_STATUS_UNSPECIFIED:
        default:
            throw std::invalid_argument("Order event has an invalid order status");
    }
}

}  // namespace

Order OrderEventMapper::toDomain(const gridx::order::v1::OrderAccepted& event) const {
    if (!event.has_market_id()) {
        throw std::invalid_argument("Order event does not contain a market ID");
    }

    if (!event.market_id().has_delivery_slot_start()) {
        throw std::invalid_argument("Order event does not contain a delivery slot start");
    }

    if (!event.has_expires_at()) {
        throw std::invalid_argument("Order event does not contain an expiry timestamp");
    }

    if (!event.has_created_at()) {
        throw std::invalid_argument("Order event does not contain a creation timestamp");
    }

    return Order{.orderId = event.order_id(),
                 .userId = event.user_id(),
                 .marketId = MarketId{.deliverySlotStart =
                                          toTimestamp(event.market_id().delivery_slot_start())},
                 .gridZone = event.grid_zone_id(),
                 .side = toDomainSide(event.side()),
                 .orderType = toDomainOrderType(event.order_type()),
                 .status = toDomainOrderStatus(event.status()),
                 .price = event.price(),
                 .quantity = event.quantity(),
                 .remainingQuantity = event.quantity(),
                 .expiresAt = toTimestamp(event.expires_at()),
                 .createdAt = toTimestamp(event.created_at())};
}

}  // namespace gridx::matching::adapters::kafka