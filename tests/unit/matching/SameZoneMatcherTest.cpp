#include <chrono>

#include <gtest/gtest.h>

#include "gridx/matching/domain/MarketId.hpp"
#include "gridx/matching/domain/Order.hpp"
#include "gridx/matching/matching/SameZoneMatcher.hpp"
#include "gridx/matching/matching/TradeManager.hpp"
#include "gridx/matching/orderbook/MarketBook.hpp"

using namespace gridx::matching;
using namespace gridx::matching::matching;
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

// Test that a BUY order fully matches a SELL order in the same grid zone.
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

// Test that a SELL order fully matches a BUY order in the same grid zone.
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

// Test that a BUY order partially matches a SELL order in the same grid zone.
TEST_F(SameZoneMatcherTest, BuyOrderPartiallyMatchesSellOrder)
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
    sellOrder->quantity = 20;
    sellOrder->remainingQuantity = 20;
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

    EXPECT_EQ(trade.quantity, 10);
    EXPECT_EQ(trade.energyPrice, 100);

    auto remainingSell =
        marketBook
            .zoneOrderBook(kZone)
            .sellBook()
            .bestOrder();

    ASSERT_NE(remainingSell, nullptr);

    EXPECT_EQ(remainingSell->orderId, sellOrder->orderId);
    EXPECT_EQ(remainingSell->remainingQuantity, 10);

    EXPECT_TRUE(
        marketBook
            .zoneOrderBook(kZone)
            .buyBook()
            .empty());
}

// Test that a SELL order partially matches a BUY order in the same grid zone.
TEST_F(SameZoneMatcherTest, BuyOrderMatchesMultipleSellOrders)
{
    auto sellOrder1 = std::make_shared<Order>();

    sellOrder1->orderId = 1;
    sellOrder1->userId = 10;
    sellOrder1->marketId = marketId;
    sellOrder1->gridZone = kZone;
    sellOrder1->side = Side::Sell;
    sellOrder1->orderType = OrderType::Limit;
    sellOrder1->status = OrderStatus::New;
    sellOrder1->price = 100;
    sellOrder1->quantity = 5;
    sellOrder1->remainingQuantity = 5;
    sellOrder1->createdAt = std::chrono::system_clock::now();
    sellOrder1->expiresAt = sellOrder1->createdAt + std::chrono::hours(1);

    auto sellOrder2 = std::make_shared<Order>();

    sellOrder2->orderId = 2;
    sellOrder2->userId = 11;
    sellOrder2->marketId = marketId;
    sellOrder2->gridZone = kZone;
    sellOrder2->side = Side::Sell;
    sellOrder2->orderType = OrderType::Limit;
    sellOrder2->status = OrderStatus::New;
    sellOrder2->price = 100;
    sellOrder2->quantity = 10;
    sellOrder2->remainingQuantity = 10;
    sellOrder2->createdAt = std::chrono::system_clock::now();
    sellOrder2->expiresAt = sellOrder2->createdAt + std::chrono::hours(1);

    marketBook.addOrder(sellOrder1);
    marketBook.addOrder(sellOrder2);

    Order buyOrder{};

    buyOrder.orderId = 3;
    buyOrder.userId = 20;
    buyOrder.marketId = marketId;
    buyOrder.gridZone = kZone;
    buyOrder.side = Side::Buy;
    buyOrder.orderType = OrderType::Limit;
    buyOrder.status = OrderStatus::New;
    buyOrder.price = 100;
    buyOrder.quantity = 15;
    buyOrder.remainingQuantity = 15;
    buyOrder.createdAt = std::chrono::system_clock::now();
    buyOrder.expiresAt = buyOrder.createdAt + std::chrono::hours(1);

    const auto trades = matcher.match(buyOrder);

    ASSERT_EQ(trades.size(), 2);

    EXPECT_EQ(trades[0].sellOrderId, sellOrder1->orderId);
    EXPECT_EQ(trades[0].quantity, 5);

    EXPECT_EQ(trades[1].sellOrderId, sellOrder2->orderId);
    EXPECT_EQ(trades[1].quantity, 10);

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


// Test that a BUY order does not match a SELL order with a higher price in the same grid zone.
TEST_F(SameZoneMatcherTest, BuyOrderDoesNotMatchHigherSellPrice)
{
    auto sellOrder = std::make_shared<Order>();

    sellOrder->orderId = 1;
    sellOrder->userId = 10;
    sellOrder->marketId = marketId;
    sellOrder->gridZone = kZone;
    sellOrder->side = Side::Sell;
    sellOrder->orderType = OrderType::Limit;
    sellOrder->status = OrderStatus::New;
    sellOrder->price = 110;
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

    EXPECT_TRUE(trades.empty());

    auto& zoneBook = marketBook.zoneOrderBook(kZone);

    EXPECT_FALSE(zoneBook.sellBook().empty());
    EXPECT_FALSE(zoneBook.buyBook().empty());

    auto restingSell = zoneBook.sellBook().bestOrder();
    auto restingBuy = zoneBook.buyBook().bestOrder();

    ASSERT_NE(restingSell, nullptr);
    ASSERT_NE(restingBuy, nullptr);

    EXPECT_EQ(restingSell->orderId, sellOrder->orderId);
    EXPECT_EQ(restingSell->remainingQuantity, 10);

    EXPECT_EQ(restingBuy->orderId, buyOrder.orderId);
    EXPECT_EQ(restingBuy->remainingQuantity, 10);
}

// Test that a BUY order with remaining quantity is added to the order book after partial matching.
TEST_F(SameZoneMatcherTest, RemainingBuyOrderIsAddedToOrderBook)
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
    sellOrder->quantity = 5;
    sellOrder->remainingQuantity = 5;
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

    EXPECT_EQ(trades.front().quantity, 5);

    auto& zoneBook = marketBook.zoneOrderBook(kZone);

    EXPECT_TRUE(zoneBook.sellBook().empty());

    auto remainingBuy = zoneBook.buyBook().bestOrder();

    ASSERT_NE(remainingBuy, nullptr);

    EXPECT_EQ(remainingBuy->orderId, buyOrder.orderId);
    EXPECT_EQ(remainingBuy->remainingQuantity, 5);
    EXPECT_EQ(remainingBuy->price, 100);

}

// Test that a trade between orders in the same grid zone has a grid fee of zero.
TEST_F(SameZoneMatcherTest, SameZoneTradeHasZeroGridFee)
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

    EXPECT_EQ(trade.buyerGridZone, kZone);
    EXPECT_EQ(trade.sellerGridZone, kZone);
    EXPECT_EQ(trade.gridFee, 0);
}
