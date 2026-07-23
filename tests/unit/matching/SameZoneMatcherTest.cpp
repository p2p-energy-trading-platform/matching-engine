#include <chrono>

#include <gtest/gtest.h>

#include "gridx/matching/domain/MarketId.hpp"
#include "gridx/matching/domain/Order.hpp"
#include "gridx/matching/matching/SameZoneMatcher.hpp"
#include "gridx/matching/matching/TradeManager.hpp"
#include "gridx/matching/orderbook/MarketBook.hpp"

using namespace gridx::matching;
using namespace gridx::matching::orderbook;

class SameZoneMatcherTest : public ::testing::Test {
protected:
    SameZoneMatcherTest()
        : marketId{
              std::chrono::system_clock::now(),
          },
          marketBook(marketId),
          matcher(marketBook, tradeManager) {}

    static constexpr GridZoneId kZone = 1;

    MarketId marketId;

    MarketBook marketBook;

    TradeManager tradeManager;

    SameZoneMatcher matcher;
};

TEST_F(SameZoneMatcherTest, BuyOrderFullyMatchesSellOrder)
{
    auto sellOrder = std::make_shared<Order>();

    sellOrder->orderId = 1;
    sellOrder->userId = 10;
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

    marketBook.addOrder(sellOrder);

    Order buyOrder{};

    buyOrder.orderId = 2;
    buyOrder.userId = 20;
    buyOrder.marketId = marketId;
    buyOrder.gridZone = kZone;
    buyOrder.side = Side::Buy;
    buyOrder.orderType = OrderType::Limit;
    buyOrder.status = OrderStatus::New;
    buyOrder.price = 100;
    buyOrder.quantity = 10;
    buyOrder.remainingQuantity = 10;
    buyOrder.createdAt = std::chrono::system_clock::now();
    buyOrder.expiresAt = buyOrder.createdAt + std::chrono::hours(1);

    const auto trades = matcher.match(buyOrder);

    ASSERT_EQ(trades.size(), 1);

    const auto& trade = trades.front();

    EXPECT_EQ(trade.buyOrderId, buyOrder.orderId);
    EXPECT_EQ(trade.sellOrderId, sellOrder->orderId);

    EXPECT_EQ(trade.buyerId, buyOrder.userId);
    EXPECT_EQ(trade.sellerId, sellOrder->userId);

    EXPECT_EQ(trade.energyPrice, 100);
    EXPECT_EQ(trade.quantity, 10);
    EXPECT_EQ(trade.gridFee, 0);

    EXPECT_TRUE(
        marketBook
            .zoneOrderBook(kZone)
            .sellBook()
            .empty());

    EXPECT_TRUE(
        marketBook
            .zoneOrderBook(kZone)
            .buyBook()
            .empty());
}


TEST_F(SameZoneMatcherTest, SellOrderFullyMatchesBuyOrder)
{
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

    marketBook.addOrder(buyOrder);

    Order sellOrder{};

    sellOrder.orderId = 2;
    sellOrder.userId = 20;
    sellOrder.marketId = marketId;
    sellOrder.gridZone = kZone;
    sellOrder.side = Side::Sell;
    sellOrder.orderType = OrderType::Limit;
    sellOrder.status = OrderStatus::New;
    sellOrder.price = 100;
    sellOrder.quantity = 10;
    sellOrder.remainingQuantity = 10;
    sellOrder.createdAt = std::chrono::system_clock::now();
    sellOrder.expiresAt = sellOrder.createdAt + std::chrono::hours(1);

    const auto trades = matcher.match(sellOrder);

    ASSERT_EQ(trades.size(), 1);

    const auto& trade = trades.front();

    EXPECT_EQ(trade.buyOrderId, buyOrder->orderId);
    EXPECT_EQ(trade.sellOrderId, sellOrder.orderId);

    EXPECT_EQ(trade.buyerId, buyOrder->userId);
    EXPECT_EQ(trade.sellerId, sellOrder.userId);

    EXPECT_EQ(trade.energyPrice, 100);
    EXPECT_EQ(trade.quantity, 10);
    EXPECT_EQ(trade.gridFee, 0);

    EXPECT_TRUE(
        marketBook
            .zoneOrderBook(kZone)
            .buyBook()
            .empty());

    EXPECT_TRUE(
        marketBook
            .zoneOrderBook(kZone)
            .sellBook()
            .empty());
}