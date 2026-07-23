#include <chrono>

#include <gtest/gtest.h>

#include "gridx/matching/domain/MarketId.hpp"
#include "gridx/matching/domain/Order.hpp"
#include "gridx/matching/expiry/ExpiryManager.hpp"
#include "gridx/matching/orderbook/MarketBook.hpp"

using namespace gridx::matching;
using namespace gridx::matching::expiry;
using namespace gridx::matching::orderbook;

class ExpiryManagerTest : public ::testing::Test {
protected:
    ExpiryManagerTest()
        : marketId{
              std::chrono::system_clock::now(),
          }
        , marketBook(marketId) {}

    static constexpr GridZoneId kZone = 1;

    MarketId marketId;

    MarketBook marketBook;

    ExpiryManager expiryManager;
};

// Test that orders are not expired before the delivery slot ends.
TEST_F(ExpiryManagerTest, DoesNotExpireOrdersBeforeDeliverySlotEnd) {
    auto buyOrder = std::make_shared<Order>();

    buyOrder->orderId = 1;
    buyOrder->userId = 10;
    buyOrder->marketId = marketId;
    buyOrder->gridZone = kZone;
    buyOrder->side = Side::Buy;
    buyOrder->orderType = OrderType::Limit;
    buyOrder->status = OrderStatus::New;
    buyOrder->price = 100;
    buyOrder->quantity = 10;
    buyOrder->remainingQuantity = 10;
    buyOrder->createdAt = std::chrono::system_clock::now();
    buyOrder->expiresAt = buyOrder->createdAt + std::chrono::hours(1);

    auto sellOrder = std::make_shared<Order>();

    sellOrder->orderId = 2;
    sellOrder->userId = 20;
    sellOrder->marketId = marketId;
    sellOrder->gridZone = kZone;
    sellOrder->side = Side::Sell;
    sellOrder->orderType = OrderType::Limit;
    sellOrder->status = OrderStatus::New;
    sellOrder->price = 100;
    sellOrder->quantity = 10;
    sellOrder->remainingQuantity = 10;
    sellOrder->createdAt = std::chrono::system_clock::now();
    sellOrder->expiresAt = sellOrder->createdAt + std::chrono::hours(1);

    marketBook.addOrder(buyOrder);
    marketBook.addOrder(sellOrder);

    expiryManager.expireOrders(
        marketBook,
        marketId.deliverySlotEnd() - std::chrono::seconds(1));

    EXPECT_EQ(buyOrder->status, OrderStatus::New);
    EXPECT_EQ(sellOrder->status, OrderStatus::New);

    EXPECT_FALSE(marketBook.zoneOrderBook(kZone).buyBook().empty());
    EXPECT_FALSE(marketBook.zoneOrderBook(kZone).sellBook().empty());
}

// Test that all orders are expired and removed once the delivery slot ends.
TEST_F(ExpiryManagerTest, ExpiresBuyAndSellOrdersWhenMarketExpires) {
    auto buyOrder = std::make_shared<Order>();

    buyOrder->orderId = 1;
    buyOrder->userId = 10;
    buyOrder->marketId = marketId;
    buyOrder->gridZone = kZone;
    buyOrder->side = Side::Buy;
    buyOrder->orderType = OrderType::Limit;
    buyOrder->status = OrderStatus::New;
    buyOrder->price = 100;
    buyOrder->quantity = 10;
    buyOrder->remainingQuantity = 10;
    buyOrder->createdAt = std::chrono::system_clock::now();
    buyOrder->expiresAt = buyOrder->createdAt + std::chrono::hours(1);

    auto sellOrder = std::make_shared<Order>();

    sellOrder->orderId = 2;
    sellOrder->userId = 20;
    sellOrder->marketId = marketId;
    sellOrder->gridZone = kZone;
    sellOrder->side = Side::Sell;
    sellOrder->orderType = OrderType::Limit;
    sellOrder->status = OrderStatus::New;
    sellOrder->price = 100;
    sellOrder->quantity = 10;
    sellOrder->remainingQuantity = 10;
    sellOrder->createdAt = std::chrono::system_clock::now();
    sellOrder->expiresAt = sellOrder->createdAt + std::chrono::hours(1);

    marketBook.addOrder(buyOrder);
    marketBook.addOrder(sellOrder);

    expiryManager.expireOrders(
        marketBook,
        marketId.deliverySlotEnd());

    EXPECT_EQ(buyOrder->status, OrderStatus::Expired);
    EXPECT_EQ(sellOrder->status, OrderStatus::Expired);

    EXPECT_TRUE(marketBook.zoneOrderBook(kZone).buyBook().empty());
    EXPECT_TRUE(marketBook.zoneOrderBook(kZone).sellBook().empty());
}

// Test that orders in multiple grid zones are expired.
TEST_F(ExpiryManagerTest, ExpiresOrdersAcrossMultipleZones) {
    constexpr GridZoneId kZone1 = 1;
    constexpr GridZoneId kZone2 = 2;

    auto buyOrder = std::make_shared<Order>();

    buyOrder->orderId = 1;
    buyOrder->userId = 10;
    buyOrder->marketId = marketId;
    buyOrder->gridZone = kZone1;
    buyOrder->side = Side::Buy;
    buyOrder->orderType = OrderType::Limit;
    buyOrder->status = OrderStatus::New;
    buyOrder->price = 100;
    buyOrder->quantity = 10;
    buyOrder->remainingQuantity = 10;
    buyOrder->createdAt = std::chrono::system_clock::now();
    buyOrder->expiresAt = buyOrder->createdAt + std::chrono::hours(1);

    auto sellOrder = std::make_shared<Order>();

    sellOrder->orderId = 2;
    sellOrder->userId = 20;
    sellOrder->marketId = marketId;
    sellOrder->gridZone = kZone2;
    sellOrder->side = Side::Sell;
    sellOrder->orderType = OrderType::Limit;
    sellOrder->status = OrderStatus::New;
    sellOrder->price = 100;
    sellOrder->quantity = 10;
    sellOrder->remainingQuantity = 10;
    sellOrder->createdAt = std::chrono::system_clock::now();
    sellOrder->expiresAt = sellOrder->createdAt + std::chrono::hours(1);

    marketBook.addOrder(buyOrder);
    marketBook.addOrder(sellOrder);

    expiryManager.expireOrders(
        marketBook,
        marketId.deliverySlotEnd());

    EXPECT_EQ(buyOrder->status, OrderStatus::Expired);
    EXPECT_EQ(sellOrder->status, OrderStatus::Expired);

    EXPECT_TRUE(marketBook.zoneOrderBook(kZone1).buyBook().empty());
    EXPECT_TRUE(marketBook.zoneOrderBook(kZone2).sellBook().empty());
}

// Test that all order books are cleared after expiry.
TEST_F(ExpiryManagerTest, ClearsOrderBooksAfterExpiry) {
    auto buyOrder = std::make_shared<Order>();

    buyOrder->orderId = 1;
    buyOrder->userId = 10;
    buyOrder->marketId = marketId;
    buyOrder->gridZone = kZone;
    buyOrder->side = Side::Buy;
    buyOrder->orderType = OrderType::Limit;
    buyOrder->status = OrderStatus::New;
    buyOrder->price = 100;
    buyOrder->quantity = 10;
    buyOrder->remainingQuantity = 10;
    buyOrder->createdAt = std::chrono::system_clock::now();
    buyOrder->expiresAt = buyOrder->createdAt + std::chrono::hours(1);

    auto sellOrder = std::make_shared<Order>();

    sellOrder->orderId = 2;
    sellOrder->userId = 20;
    sellOrder->marketId = marketId;
    sellOrder->gridZone = kZone;
    sellOrder->side = Side::Sell;
    sellOrder->orderType = OrderType::Limit;
    sellOrder->status = OrderStatus::New;
    sellOrder->price = 100;
    sellOrder->quantity = 10;
    sellOrder->remainingQuantity = 10;
    sellOrder->createdAt = std::chrono::system_clock::now();
    sellOrder->expiresAt = sellOrder->createdAt + std::chrono::hours(1);

    marketBook.addOrder(buyOrder);
    marketBook.addOrder(sellOrder);

    ASSERT_FALSE(marketBook.zoneOrderBook(kZone).buyBook().empty());
    ASSERT_FALSE(marketBook.zoneOrderBook(kZone).sellBook().empty());

    expiryManager.expireOrders(
        marketBook,
        marketId.deliverySlotEnd());

    EXPECT_TRUE(marketBook.zoneOrderBook(kZone).buyBook().empty());
    EXPECT_TRUE(marketBook.zoneOrderBook(kZone).sellBook().empty());

    EXPECT_EQ(buyOrder->status, OrderStatus::Expired);
    EXPECT_EQ(sellOrder->status, OrderStatus::Expired);
}

