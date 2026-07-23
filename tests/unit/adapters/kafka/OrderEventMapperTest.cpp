#include <gtest/gtest.h>

#include <chrono>

#include "gridx/matching/adapters/kafka/OrderEventMapper.hpp"
#include "gridx/order/v1/order_events.pb.h"

namespace {

namespace proto = gridx::order::v1;

using gridx::matching::OrderStatus;
using gridx::matching::OrderType;
using gridx::matching::Side;
using gridx::matching::Timestamp;
using gridx::matching::adapters::kafka::OrderEventMapper;

class OrderEventMapperTest : public ::testing::Test {
protected:
    OrderEventMapper mapper;

    static proto::OrderAccepted makeValidEvent() {
        proto::OrderAccepted event;

        event.set_order_id(101);
        event.set_user_id(202);
        event.set_grid_zone_id(3);

        event.set_side(proto::SIDE_BUY);
        event.set_order_type(proto::ORDER_TYPE_LIMIT);
        event.set_status(proto::ORDER_STATUS_NEW);

        event.set_price(5000);
        event.set_quantity(100);

        auto* deliverySlot = event.mutable_market_id()->mutable_delivery_slot_start();

        deliverySlot->set_seconds(1'700'000'000);
        deliverySlot->set_nanos(123'000'000);

        auto* createdAt = event.mutable_created_at();
        createdAt->set_seconds(1'700'000'100);
        createdAt->set_nanos(456'000'000);

        auto* expiresAt = event.mutable_expires_at();
        expiresAt->set_seconds(1'700'003'600);
        expiresAt->set_nanos(789'000'000);

        return event;
    }

    static Timestamp expectedTimestamp(std::int64_t seconds, std::int32_t nanoseconds) {
        using namespace std::chrono;

        const auto duration = std::chrono::seconds{seconds} + std::chrono::nanoseconds{nanoseconds};

        return std::chrono::system_clock::time_point{
            duration_cast<std::chrono::system_clock::duration>(duration)};
    }
};

}  // namespace

TEST_F(OrderEventMapperTest, MapsValidEventDomainOrder) {
    const auto event = makeValidEvent();

    const auto order = mapper.toDomain(event);

    EXPECT_EQ(order.orderId, 101);
    EXPECT_EQ(order.userId, 202);
    EXPECT_EQ(order.gridZone, 3);

    EXPECT_EQ(order.side, Side::Buy);
    EXPECT_EQ(order.orderType, OrderType::Limit);
    EXPECT_EQ(order.status, OrderStatus::New);

    EXPECT_EQ(order.price, 5000);
    EXPECT_EQ(order.quantity, 100);
    EXPECT_EQ(order.remainingQuantity, 100);

    EXPECT_EQ(order.marketId.deliverySlotStart, expectedTimestamp(1'700'000'000, 123'000'000));

    EXPECT_EQ(order.createdAt, expectedTimestamp(1'700'000'100, 456'000'000));

    EXPECT_EQ(order.expiresAt, expectedTimestamp(1'700'003'600, 789'000'000));
}

TEST_F(OrderEventMapperTest, MapsSellSide) {
    auto event = makeValidEvent();

    event.set_side(proto::SIDE_SELL);

    const auto order = mapper.toDomain(event);

    EXPECT_EQ(order.side, Side::Sell);
}

TEST_F(OrderEventMapperTest, MapsNewStatus) {
    auto event = makeValidEvent();
    event.set_status(proto::ORDER_STATUS_NEW);

    const auto order = mapper.toDomain(event);

    EXPECT_EQ(order.status, OrderStatus::New);
}

TEST_F(OrderEventMapperTest, MapsPartiallyFilledStatus) {
    auto event = makeValidEvent();
    event.set_status(proto::ORDER_STATUS_PARTIALLY_FILLED);

    const auto order = mapper.toDomain(event);

    EXPECT_EQ(order.status, OrderStatus::PartiallyFilled);
}

TEST_F(OrderEventMapperTest, MapsFilledStatus) {
    auto event = makeValidEvent();
    event.set_status(proto::ORDER_STATUS_FILLED);

    const auto order = mapper.toDomain(event);

    EXPECT_EQ(order.status, OrderStatus::Filled);
}

TEST_F(OrderEventMapperTest, MapsCancelledStatus) {
    auto event = makeValidEvent();
    event.set_status(proto::ORDER_STATUS_CANCELLED);

    const auto order = mapper.toDomain(event);

    EXPECT_EQ(order.status, OrderStatus::Cancelled);
}

TEST_F(OrderEventMapperTest, MapsExpiredStatus) {
    auto event = makeValidEvent();
    event.set_status(proto::ORDER_STATUS_EXPIRED);

    const auto order = mapper.toDomain(event);

    EXPECT_EQ(order.status, OrderStatus::Expired);
}

TEST_F(OrderEventMapperTest, RejectsMissingMarketId) {
    auto event = makeValidEvent();
    event.clear_market_id();

    EXPECT_THROW(static_cast<void>(mapper.toDomain(event)), std::invalid_argument);
}

TEST_F(OrderEventMapperTest, RejectsMissingDeliverySlot) {
    auto event = makeValidEvent();
    event.mutable_market_id()->clear_delivery_slot_start();

    EXPECT_THROW(static_cast<void>(mapper.toDomain(event)), std::invalid_argument);
}

TEST_F(OrderEventMapperTest, RejectsMissingExpiryTimestamp) {
    auto event = makeValidEvent();
    event.clear_expires_at();

    EXPECT_THROW(static_cast<void>(mapper.toDomain(event)), std::invalid_argument);
}

TEST_F(OrderEventMapperTest, RejectsMissingCreationTimestamp) {
    auto event = makeValidEvent();
    event.clear_created_at();

    EXPECT_THROW(static_cast<void>(mapper.toDomain(event)), std::invalid_argument);
}

TEST_F(OrderEventMapperTest, RejectsUnspecifiedSide) {
    auto event = makeValidEvent();
    event.set_side(proto::SIDE_UNSPECIFIED);

    EXPECT_THROW(static_cast<void>(mapper.toDomain(event)), std::invalid_argument);
}