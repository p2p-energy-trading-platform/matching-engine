#include <chrono>

#include <gtest/gtest.h>

#include "gridx/matching/expiry/ExpiryManager.hpp"
#include "gridx/matching/orderbook/MarketBook.hpp"
#include "support/TestSupport.hpp"

using namespace gridx::matching;
using namespace gridx::matching::expiry;
using namespace gridx::matching::orderbook;
using namespace gridx::matching::test_support;

class ExpiryManagerTest : public ::testing::Test {
protected:
    ExpiryManagerTest()
        : marketId{
              std::chrono::system_clock::now(),
          }
        , marketBook(marketId) {}

    OrderPtr makeOrder(OrderId orderId, UserId userId, GridZoneId gridZone, Side side, Price price,
                       Quantity quantity, const Timestamp& createdAt = fixedTimestamp()) const {
        return OrderBuilder{}
            .withOrderId(orderId)
            .withUserId(userId)
            .withMarketId(marketId)
            .withGridZone(gridZone)
            .withSide(side)
            .withOrderType(OrderType::Limit)
            .withStatus(OrderStatus::New)
            .withPrice(price)
            .withQuantity(quantity)
            .withCreatedAt(createdAt)
            .withExpiresAt(after(createdAt, std::chrono::hours{1}))
            .buildPtr();
    }

    static constexpr GridZoneId kZone = 1;

    MarketId marketId;

    MarketBook marketBook;

    ExpiryManager expiryManager;
};

// Test that orders are not expired before the delivery slot ends.
TEST_F(ExpiryManagerTest, DoesNotExpireOrdersBeforeDeliverySlotEnd) {
    auto buyOrder = makeOrder(1, 10, kZone, Side::Buy, 100, 10, std::chrono::system_clock::now());

    auto sellOrder = makeOrder(2, 20, kZone, Side::Sell, 100, 10, std::chrono::system_clock::now());

    marketBook.addOrder(buyOrder);
    marketBook.addOrder(sellOrder);

    expiryManager.expireOrders(marketBook, marketId.deliverySlotEnd() - std::chrono::seconds(1));

    EXPECT_EQ(buyOrder->status, OrderStatus::New);
    EXPECT_EQ(sellOrder->status, OrderStatus::New);

    EXPECT_FALSE(marketBook.zoneOrderBook(kZone).empty(Side::Buy));
    EXPECT_FALSE(marketBook.zoneOrderBook(kZone).empty(Side::Sell));
}

// Test that all orders are expired and removed once the delivery slot ends.
TEST_F(ExpiryManagerTest, ExpiresBuyAndSellOrdersWhenMarketExpires) {
    auto buyOrder = makeOrder(1, 10, kZone, Side::Buy, 100, 10, std::chrono::system_clock::now());

    auto sellOrder = makeOrder(2, 20, kZone, Side::Sell, 100, 10, std::chrono::system_clock::now());

    marketBook.addOrder(buyOrder);
    marketBook.addOrder(sellOrder);

    expiryManager.expireOrders(marketBook, marketId.deliverySlotEnd());

    EXPECT_EQ(buyOrder->status, OrderStatus::Expired);
    EXPECT_EQ(sellOrder->status, OrderStatus::Expired);

    EXPECT_TRUE(marketBook.zoneOrderBook(kZone).empty(Side::Buy));
    EXPECT_TRUE(marketBook.zoneOrderBook(kZone).empty(Side::Sell));
}

// Test that orders in multiple grid zones are expired.
TEST_F(ExpiryManagerTest, ExpiresOrdersAcrossMultipleZones) {
    constexpr GridZoneId kZone1 = 1;
    constexpr GridZoneId kZone2 = 2;

    auto buyOrder = makeOrder(1, 10, kZone1, Side::Buy, 100, 10, std::chrono::system_clock::now());

    auto sellOrder =
        makeOrder(2, 20, kZone2, Side::Sell, 100, 10, std::chrono::system_clock::now());

    marketBook.addOrder(buyOrder);
    marketBook.addOrder(sellOrder);

    expiryManager.expireOrders(marketBook, marketId.deliverySlotEnd());

    EXPECT_EQ(buyOrder->status, OrderStatus::Expired);
    EXPECT_EQ(sellOrder->status, OrderStatus::Expired);

    EXPECT_TRUE(marketBook.zoneOrderBook(kZone1).empty(Side::Buy));
    EXPECT_TRUE(marketBook.zoneOrderBook(kZone2).empty(Side::Sell));
}

// Test that all order books are cleared after expiry.
TEST_F(ExpiryManagerTest, ClearsOrderBooksAfterExpiry) {
    auto buyOrder = makeOrder(1, 10, kZone, Side::Buy, 100, 10, std::chrono::system_clock::now());

    auto sellOrder = makeOrder(2, 20, kZone, Side::Sell, 100, 10, std::chrono::system_clock::now());

    marketBook.addOrder(buyOrder);
    marketBook.addOrder(sellOrder);

    ASSERT_FALSE(marketBook.zoneOrderBook(kZone).empty(Side::Buy));
    ASSERT_FALSE(marketBook.zoneOrderBook(kZone).empty(Side::Sell));

    expiryManager.expireOrders(marketBook, marketId.deliverySlotEnd());

    EXPECT_TRUE(marketBook.zoneOrderBook(kZone).empty(Side::Buy));
    EXPECT_TRUE(marketBook.zoneOrderBook(kZone).empty(Side::Sell));

    EXPECT_EQ(buyOrder->status, OrderStatus::Expired);
    EXPECT_EQ(sellOrder->status, OrderStatus::Expired);
}
