#include <chrono>

#include <gtest/gtest.h>

#include "gridx/matching/domain/MarketId.hpp"
#include "gridx/matching/validation/OrderValidator.hpp"

using namespace gridx::matching;
using namespace gridx::matching::validation;

class OrderValidatorTest : public ::testing::Test
{
protected:
    OrderValidator validator;

    Order createValidOrder() const
    {
        Order order{};

        order.orderId = 1;
        order.userId = 1;

        order.marketId.deliverySlotStart =
            std::chrono::system_clock::now();

        order.gridZone = 1;

        order.side = Side::Buy;
        order.orderType = OrderType::Limit;
        order.status = OrderStatus::New;

        order.price = 100.0;
        order.quantity = 10.0;
        order.remainingQuantity = 10.0;

        order.createdAt = std::chrono::system_clock::now();
        order.expiresAt = order.createdAt + std::chrono::minutes(30);

        return order;
    }
};

TEST_F(OrderValidatorTest, AcceptsValidOrder)
{
    const auto order = createValidOrder();

    const auto result = validator.validate(order);

    EXPECT_TRUE(result.valid);
    EXPECT_EQ(result.error, ValidationError::None);
}

TEST_F(OrderValidatorTest, RejectsInvalidOrderId)
{
    auto order = createValidOrder();
    order.orderId = 0;

    const auto result = validator.validate(order);

    EXPECT_FALSE(result.valid);
    EXPECT_EQ(result.error, ValidationError::InvalidOrderId);
}

TEST_F(OrderValidatorTest, RejectsInvalidUserId)
{
    auto order = createValidOrder();
    order.userId = 0;

    const auto result = validator.validate(order);

    EXPECT_FALSE(result.valid);
    EXPECT_EQ(result.error, ValidationError::InvalidUserId);
}

TEST_F(OrderValidatorTest, RejectsInvalidOrderType)
{
    auto order = createValidOrder();
    order.orderType = static_cast<OrderType>(999);

    const auto result = validator.validate(order);

    EXPECT_FALSE(result.valid);
    EXPECT_EQ(result.error, ValidationError::InvalidOrderType);
}

TEST_F(OrderValidatorTest, RejectsInvalidPrice)
{
    auto order = createValidOrder();
    order.price = 0.0;

    const auto result = validator.validate(order);

    EXPECT_FALSE(result.valid);
    EXPECT_EQ(result.error, ValidationError::InvalidPrice);
}

TEST_F(OrderValidatorTest, RejectsInvalidQuantity)
{
    auto order = createValidOrder();
    order.quantity = 0.0;

    const auto result = validator.validate(order);

    EXPECT_FALSE(result.valid);
    EXPECT_EQ(result.error, ValidationError::InvalidQuantity);
}

TEST_F(OrderValidatorTest, RejectsNegativeRemainingQuantity)
{
    auto order = createValidOrder();
    order.remainingQuantity = -1.0;

    const auto result = validator.validate(order);

    EXPECT_FALSE(result.valid);
    EXPECT_EQ(result.error, ValidationError::InvalidRemainingQuantity);
}

TEST_F(OrderValidatorTest, RejectsRemainingQuantityGreaterThanQuantity)
{
    auto order = createValidOrder();
    order.remainingQuantity = order.quantity + 1.0;

    const auto result = validator.validate(order);

    EXPECT_FALSE(result.valid);
    EXPECT_EQ(result.error, ValidationError::InvalidRemainingQuantity);
}

TEST_F(OrderValidatorTest, RejectsExpiredOrder)
{
    auto order = createValidOrder();
    order.expiresAt =
        std::chrono::system_clock::now() - std::chrono::seconds(1);

    const auto result = validator.validate(order);

    EXPECT_FALSE(result.valid);
    EXPECT_EQ(result.error, ValidationError::OrderExpired);
}