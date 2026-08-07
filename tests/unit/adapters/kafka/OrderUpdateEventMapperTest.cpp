#include <chrono>

#include <gtest/gtest.h>

#include "gridx/matching/adapters/kafka/OrderUpdateEventMapper.hpp"

namespace gridx::matching::adapters::kafka {

class OrderUpdateEventMapperTest : public ::testing::Test {
protected:
    OrderUpdateEventMapper mapper;
};

TEST_F(OrderUpdateEventMapperTest, MapsFilledOrderUpdate) {
    using namespace std::chrono;

    auto order = std::make_shared<Order>();

    order->orderId = 100;
    order->createdAt = system_clock::now();

    OrderUpdate update{};

    update.order = order;
    update.remainingQuantity = 0;
    update.status = OrderStatus::Filled;
    update.action = OrderUpdateAction::Remove;

    const auto event = mapper.toProtobuf(update);

    EXPECT_EQ(event.order_id(), order->orderId);
    EXPECT_EQ(event.remaining_quantity(), 0);

    EXPECT_EQ(event.status(), gridx::order::v1::ORDER_STATUS_FILLED);

    EXPECT_EQ(event.event_type(), gridx::order::v1::ORDER_UPDATE_TYPE_FILLED);

    EXPECT_TRUE(event.has_updated_at());
}

TEST_F(OrderUpdateEventMapperTest, MapsPartiallyFilledOrderUpdate) {
    auto order = std::make_shared<Order>();

    order->orderId = 200;

    OrderUpdate update{};

    update.order = order;
    update.remainingQuantity = 25;
    update.status = OrderStatus::PartiallyFilled;
    update.action = OrderUpdateAction::Update;

    const auto event = mapper.toProtobuf(update);

    EXPECT_EQ(event.order_id(), order->orderId);
    EXPECT_EQ(event.remaining_quantity(), 25);

    EXPECT_EQ(event.status(), gridx::order::v1::ORDER_STATUS_PARTIALLY_FILLED);

    EXPECT_EQ(event.event_type(), gridx::order::v1::ORDER_UPDATE_TYPE_PARTIALLY_FILLED);
}

TEST_F(OrderUpdateEventMapperTest, MapsCancelledOrderUpdate) {
    auto order = std::make_shared<Order>();

    order->orderId = 300;

    OrderUpdate update{};

    update.order = order;
    update.remainingQuantity = 10;
    update.status = OrderStatus::Cancelled;
    update.action = OrderUpdateAction::Remove;

    const auto event = mapper.toProtobuf(update);

    EXPECT_EQ(event.status(), gridx::order::v1::ORDER_STATUS_CANCELLED);

    EXPECT_EQ(event.event_type(), gridx::order::v1::ORDER_UPDATE_TYPE_CANCELLED);
}

TEST_F(OrderUpdateEventMapperTest, MapsExpiredOrderUpdate) {
    auto order = std::make_shared<Order>();

    order->orderId = 400;

    OrderUpdate update{};

    update.order = order;
    update.remainingQuantity = 15;
    update.status = OrderStatus::Expired;
    update.action = OrderUpdateAction::Remove;

    const auto event = mapper.toProtobuf(update);

    EXPECT_EQ(event.status(), gridx::order::v1::ORDER_STATUS_EXPIRED);

    EXPECT_EQ(event.event_type(), gridx::order::v1::ORDER_UPDATE_TYPE_EXPIRED);
}

TEST_F(OrderUpdateEventMapperTest, ThrowsWhenOrderPointerIsNull) {
    OrderUpdate update{};

    update.order = nullptr;
    update.status = OrderStatus::Filled;
    update.action = OrderUpdateAction::Remove;

    EXPECT_THROW(mapper.toProtobuf(update), std::invalid_argument);
}

}  // namespace gridx::matching::adapters::kafka