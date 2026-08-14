#include <gtest/gtest.h>

#include "gridx/matching/validation/OrderValidator.hpp"
#include "support/TestSupport.hpp"

using namespace gridx::matching;
using namespace gridx::matching::validation;
using namespace gridx::matching::test_support;

class OrderValidatorTest : public ::testing::Test {
protected:
    OrderValidator validator;

    Order createValidOrder() const {
        return OrderBuilder{}
            .withOrderId(1)
            .withUserId(1)
            .withMarketId(makeMarketId())
            .withGridZone(1)
            .buy()
            .withOrderType(OrderType::Limit)
            .withStatus(OrderStatus::New)
            .withPrice(100.0)
            .withQuantity(10.0)
            .withCreatedAt(fixedTimestamp())
            .withExpiresAt(after(fixedTimestamp(), std::chrono::minutes{30}))
            .build();
    }
};

TEST_F(OrderValidatorTest, AcceptsValidOrder) {
    const auto order = createValidOrder();

    const auto result = validator.validate(order);

    EXPECT_TRUE(result.valid);
    EXPECT_EQ(result.error, ValidationError::None);
}

TEST_F(OrderValidatorTest, RejectsInvalidOrderId) {
    auto order = createValidOrder();
    order.orderId = 0;

    const auto result = validator.validate(order);

    EXPECT_FALSE(result.valid);
    EXPECT_EQ(result.error, ValidationError::InvalidOrderId);
}

TEST_F(OrderValidatorTest, RejectsInvalidUserId) {
    auto order = createValidOrder();
    order.userId = 0;

    const auto result = validator.validate(order);

    EXPECT_FALSE(result.valid);
    EXPECT_EQ(result.error, ValidationError::InvalidUserId);
}

TEST_F(OrderValidatorTest, RejectsInvalidOrderType) {
    auto order = createValidOrder();
    order.orderType = static_cast<OrderType>(999);

    const auto result = validator.validate(order);

    EXPECT_FALSE(result.valid);
    EXPECT_EQ(result.error, ValidationError::InvalidOrderType);
}

TEST_F(OrderValidatorTest, RejectsInvalidPrice) {
    auto order = createValidOrder();
    order.price = 0.0;

    const auto result = validator.validate(order);

    EXPECT_FALSE(result.valid);
    EXPECT_EQ(result.error, ValidationError::InvalidPrice);
}

TEST_F(OrderValidatorTest, RejectsInvalidQuantity) {
    auto order = createValidOrder();
    order.quantity = 0.0;

    const auto result = validator.validate(order);

    EXPECT_FALSE(result.valid);
    EXPECT_EQ(result.error, ValidationError::InvalidQuantity);
}

TEST_F(OrderValidatorTest, RejectsNegativeRemainingQuantity) {
    auto order = createValidOrder();
    order.remainingQuantity = -1.0;

    const auto result = validator.validate(order);

    EXPECT_FALSE(result.valid);
    EXPECT_EQ(result.error, ValidationError::InvalidRemainingQuantity);
}

TEST_F(OrderValidatorTest, RejectsRemainingQuantityGreaterThanQuantity) {
    auto order = createValidOrder();
    order.remainingQuantity = order.quantity + 1.0;

    const auto result = validator.validate(order);

    EXPECT_FALSE(result.valid);
    EXPECT_EQ(result.error, ValidationError::InvalidRemainingQuantity);
}

TEST_F(OrderValidatorTest, RejectsExpiredOrder) {
    auto order = createValidOrder();
    order.expiresAt = std::chrono::system_clock::now() - std::chrono::seconds(1);

    const auto result = validator.validate(order);

    EXPECT_FALSE(result.valid);
    EXPECT_EQ(result.error, ValidationError::OrderExpired);
}