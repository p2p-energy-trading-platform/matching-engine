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

    OrderPtr makeOrder(OrderId orderId, std::uint64_t userId, GridZoneId gridZone, Side side,
                       Price price, Quantity quantity,
                       const Timestamp& createdAt = fixedTimestamp()) const {
        return OrderBuilder{}
            .withOrderId(orderId)
            .withUserId(testUserId(userId))
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

// Test that nothing is reported before the delivery slot ends.
TEST_F(ExpiryManagerTest, ReturnsNoUpdatesBeforeDeliverySlotEnd) {
    marketBook.addOrder(makeOrder(1, 10, kZone, Side::Buy, 100, 10));

    const auto updates = expiryManager.expireOrders(
        marketBook, marketId.deliverySlotEnd() - std::chrono::seconds(1));

    EXPECT_TRUE(updates.empty());
}

// Test that each expired order is reported as a Remove/Expired update carrying the order.
TEST_F(ExpiryManagerTest, ReturnsExpiredUpdateForEachOrder) {
    auto buyOrder = makeOrder(1, 10, kZone, Side::Buy, 100, 10);
    auto sellOrder = makeOrder(2, 20, kZone, Side::Sell, 100, 10);
    sellOrder->remainingQuantity = 4;  // partially filled before expiry

    marketBook.addOrder(buyOrder);
    marketBook.addOrder(sellOrder);

    const auto updates = expiryManager.expireOrders(marketBook, marketId.deliverySlotEnd());

    ASSERT_EQ(updates.size(), 2U);

    for (const auto& update : updates) {
        EXPECT_EQ(update.status, OrderStatus::Expired);
        EXPECT_EQ(update.action, OrderUpdateAction::Remove);
        EXPECT_EQ(update.remainingQuantity, update.order->remainingQuantity);
    }

    EXPECT_EQ(updates[0].order, buyOrder);
    EXPECT_EQ(updates[1].order, sellOrder);
    EXPECT_EQ(updates[1].remainingQuantity, 4);
}

// Test that expireMarkets only expires ended markets and removes their Market Books.
TEST(ExpiryManagerMarketsTest, ExpiresOnlyEndedMarketsAndRemovesThem) {
    const Timestamp now = std::chrono::system_clock::now();

    const MarketId endedMarket{now - std::chrono::hours{1}};
    const MarketId openMarket{now};

    MarketBookManager marketBookManager;

    marketBookManager.getOrCreate(endedMarket)
        .addOrder(OrderBuilder{}.withOrderId(1).withMarketId(endedMarket).buildPtr());
    marketBookManager.getOrCreate(openMarket)
        .addOrder(OrderBuilder{}.withOrderId(2).withMarketId(openMarket).buildPtr());

    const ExpiryManager expiryManager;
    const auto updates = expiryManager.expireMarkets(marketBookManager, now);

    ASSERT_EQ(updates.size(), 1U);
    EXPECT_EQ(updates[0].order->orderId, 1U);

    EXPECT_EQ(marketBookManager.find(endedMarket), nullptr);
    EXPECT_NE(marketBookManager.find(openMarket), nullptr);
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
