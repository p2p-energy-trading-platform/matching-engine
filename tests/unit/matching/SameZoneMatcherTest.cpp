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
          }
        , marketBook(marketId)
        , matcher(marketBook, tradeManager) {}

    static constexpr GridZoneId kZone = 1;

    MarketId marketId;

    MarketBook marketBook;

    TradeManager tradeManager;

    SameZoneMatcher matcher;
};

// Test that a BUY order fully matches a SELL order in the same grid zone.
TEST_F(SameZoneMatcherTest, BuyOrderFullyMatchesSellOrder) {
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

    const auto result = matcher.match(buyOrder);

    ASSERT_EQ(result.trades.size(), 1);

    const auto& trade = result.trades.front();

    EXPECT_EQ(trade.buyOrderId, buyOrder.orderId);
    EXPECT_EQ(trade.sellOrderId, sellOrder->orderId);

    EXPECT_EQ(trade.buyerId, buyOrder.userId);
    EXPECT_EQ(trade.sellerId, sellOrder->userId);

    EXPECT_EQ(trade.energyPrice, 100);
    EXPECT_EQ(trade.quantity, 10);
    EXPECT_EQ(trade.gridFee, 0);

    ASSERT_EQ(result.orderUpdates.size(), 1);

    const auto& update = result.orderUpdates.front();
    ASSERT_NE(update.order, nullptr);
    EXPECT_EQ(update.order->orderId, sellOrder->orderId);
    EXPECT_EQ(update.remainingQuantity, 0);
    EXPECT_EQ(update.status, OrderStatus::Filled);
    EXPECT_EQ(update.action, OrderUpdateAction::Remove);

    EXPECT_EQ(result.incomingOrderToInsert, nullptr);
}

// Test that a SELL order fully matches a BUY order in the same grid zone.
TEST_F(SameZoneMatcherTest, SellOrderFullyMatchesBuyOrder) {
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

    const auto result = matcher.match(sellOrder);

    ASSERT_EQ(result.trades.size(), 1);

    const auto& trade = result.trades.front();

    EXPECT_EQ(trade.buyOrderId, buyOrder->orderId);
    EXPECT_EQ(trade.sellOrderId, sellOrder.orderId);

    EXPECT_EQ(trade.buyerId, buyOrder->userId);
    EXPECT_EQ(trade.sellerId, sellOrder.userId);

    EXPECT_EQ(trade.energyPrice, 100);
    EXPECT_EQ(trade.quantity, 10);
    EXPECT_EQ(trade.gridFee, 0);

    ASSERT_EQ(result.orderUpdates.size(), 1);

    const auto& update = result.orderUpdates.front();

    ASSERT_NE(update.order, nullptr);

    EXPECT_EQ(update.order->orderId, buyOrder->orderId);
    EXPECT_EQ(update.remainingQuantity, 0);
    EXPECT_EQ(update.status, OrderStatus::Filled);
    EXPECT_EQ(update.action, OrderUpdateAction::Remove);

    EXPECT_EQ(result.incomingOrderToInsert, nullptr);
}

// Test that a BUY order partially matches a SELL order in the same grid zone.
TEST_F(SameZoneMatcherTest, BuyOrderPartiallyMatchesSellOrder) {
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

    const auto result = matcher.match(buyOrder);

    ASSERT_EQ(result.trades.size(), 1);

    const auto& trade = result.trades.front();

    EXPECT_EQ(trade.quantity, 10);
    EXPECT_EQ(trade.energyPrice, 100);

    ASSERT_EQ(result.orderUpdates.size(), 1);
    const auto& update = result.orderUpdates.front();

    ASSERT_NE(update.order, nullptr);
    EXPECT_EQ(update.order->orderId, sellOrder->orderId);
    EXPECT_EQ(update.remainingQuantity, 10);
    EXPECT_EQ(update.status, OrderStatus::PartiallyFilled);
    EXPECT_EQ(update.action, OrderUpdateAction::Update);

    EXPECT_EQ(result.incomingOrderToInsert, nullptr);
}

// Test that a BUY order matches multiple SELL orders in the same grid zone.
TEST_F(SameZoneMatcherTest, BuyOrderMatchesMultipleSellOrders) {
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
    const auto result = matcher.match(buyOrder);

    ASSERT_EQ(result.trades.size(), 2);

    EXPECT_EQ(result.trades[0].sellOrderId, sellOrder1->orderId);
    EXPECT_EQ(result.trades[0].quantity, 5);

    EXPECT_EQ(result.trades[1].sellOrderId, sellOrder2->orderId);
    EXPECT_EQ(result.trades[1].quantity, 10);

    ASSERT_EQ(result.orderUpdates.size(), 2);

    const auto& update1 = result.orderUpdates.front();
    ASSERT_NE(update1.order, nullptr);
    EXPECT_EQ(update1.order->orderId, sellOrder1->orderId);
    EXPECT_EQ(update1.remainingQuantity, 0);
    EXPECT_EQ(update1.status, OrderStatus::Filled);
    EXPECT_EQ(update1.action, OrderUpdateAction::Remove);

    const auto& update2 = result.orderUpdates.back();
    ASSERT_NE(update2.order, nullptr);
    EXPECT_EQ(update2.order->orderId, sellOrder2->orderId);
    EXPECT_EQ(update2.remainingQuantity, 0);
    EXPECT_EQ(update2.status, OrderStatus::Filled);
    EXPECT_EQ(update2.action, OrderUpdateAction::Remove);

    EXPECT_EQ(result.incomingOrderToInsert, nullptr);
}

// Test that a BUY order does not match a SELL order with a higher price in the same grid zone.
TEST_F(SameZoneMatcherTest, BuyOrderDoesNotMatchHigherSellPrice) {
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

    const auto result = matcher.match(buyOrder);

    EXPECT_TRUE(result.trades.empty());

    ASSERT_TRUE(result.orderUpdates.empty());

    ASSERT_NE(result.incomingOrderToInsert, nullptr);

    EXPECT_EQ(result.incomingOrderToInsert->orderId, buyOrder.orderId);
    EXPECT_EQ(result.incomingOrderToInsert->remainingQuantity, 10);
    EXPECT_EQ(result.incomingOrderToInsert->price, 100);
    EXPECT_EQ(result.incomingOrderToInsert->side, Side::Buy);
}

// Test that a BUY order with remaining quantity is added to the order book after partial matching.
TEST_F(SameZoneMatcherTest, RemainingBuyOrderIsAddedToOrderBook) {
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

    const auto result = matcher.match(buyOrder);

    ASSERT_EQ(result.trades.size(), 1);

    EXPECT_EQ(result.trades.front().quantity, 5);

    ASSERT_EQ(result.orderUpdates.size(), 1);

    const auto& update = result.orderUpdates.front();
    ASSERT_NE(update.order, nullptr);
    EXPECT_EQ(update.order->orderId, sellOrder->orderId);
    EXPECT_EQ(update.remainingQuantity, 0);
    EXPECT_EQ(update.status, OrderStatus::Filled);
    EXPECT_EQ(update.action, OrderUpdateAction::Remove);

    ASSERT_NE(result.incomingOrderToInsert, nullptr);

    EXPECT_EQ(result.incomingOrderToInsert->orderId, buyOrder.orderId);
    EXPECT_EQ(result.incomingOrderToInsert->remainingQuantity, 5);
    EXPECT_EQ(result.incomingOrderToInsert->price, 100);
    EXPECT_EQ(result.incomingOrderToInsert->side, Side::Buy);
}

// Test that a trade between orders in the same grid zone has a grid fee of zero.
TEST_F(SameZoneMatcherTest, SameZoneTradeHasZeroGridFee) {
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
    const auto result = matcher.match(buyOrder);

    ASSERT_EQ(result.trades.size(), 1);

    const auto& trade = result.trades.front();

    EXPECT_EQ(trade.buyerGridZone, kZone);
    EXPECT_EQ(trade.sellerGridZone, kZone);
    EXPECT_EQ(trade.gridFee, 0);
}

// Test acceptance example: BUY 100 matches SELL 40 + SELL 30 + SELL 50.
TEST_F(SameZoneMatcherTest, BuyOneHundredMatchesThreeSellOrdersAndLeavesRemainderOnLastOrder) {
    auto sellOrder1 = std::make_shared<Order>();
    sellOrder1->orderId = 1;
    sellOrder1->userId = 10;
    sellOrder1->marketId = marketId;
    sellOrder1->gridZone = kZone;
    sellOrder1->side = Side::Sell;
    sellOrder1->orderType = OrderType::Limit;
    sellOrder1->status = OrderStatus::New;
    sellOrder1->price = 100;
    sellOrder1->quantity = 40;
    sellOrder1->remainingQuantity = 40;
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
    sellOrder2->quantity = 30;
    sellOrder2->remainingQuantity = 30;
    sellOrder2->createdAt = std::chrono::system_clock::now();
    sellOrder2->expiresAt = sellOrder2->createdAt + std::chrono::hours(1);

    auto sellOrder3 = std::make_shared<Order>();
    sellOrder3->orderId = 3;
    sellOrder3->userId = 12;
    sellOrder3->marketId = marketId;
    sellOrder3->gridZone = kZone;
    sellOrder3->side = Side::Sell;
    sellOrder3->orderType = OrderType::Limit;
    sellOrder3->status = OrderStatus::New;
    sellOrder3->price = 100;
    sellOrder3->quantity = 50;
    sellOrder3->remainingQuantity = 50;
    sellOrder3->createdAt = std::chrono::system_clock::now();
    sellOrder3->expiresAt = sellOrder3->createdAt + std::chrono::hours(1);

    marketBook.addOrder(sellOrder1);
    marketBook.addOrder(sellOrder2);
    marketBook.addOrder(sellOrder3);

    Order buyOrder{};
    buyOrder.orderId = 100;
    buyOrder.userId = 20;
    buyOrder.marketId = marketId;
    buyOrder.gridZone = kZone;
    buyOrder.side = Side::Buy;
    buyOrder.orderType = OrderType::Limit;
    buyOrder.status = OrderStatus::New;
    buyOrder.price = 100;
    buyOrder.quantity = 100;
    buyOrder.remainingQuantity = 100;
    buyOrder.createdAt = std::chrono::system_clock::now();
    buyOrder.expiresAt = buyOrder.createdAt + std::chrono::hours(1);

    const auto result = matcher.match(buyOrder);

    ASSERT_EQ(result.trades.size(), 3);
    EXPECT_EQ(result.trades[0].quantity, 40);
    EXPECT_EQ(result.trades[1].quantity, 30);
    EXPECT_EQ(result.trades[2].quantity, 30);

    ASSERT_EQ(result.orderUpdates.size(), 3);

    const auto& update1 = result.orderUpdates[0];
    EXPECT_EQ(update1.order->orderId, sellOrder1->orderId);
    EXPECT_EQ(update1.remainingQuantity, 0);
    EXPECT_EQ(update1.status, OrderStatus::Filled);
    EXPECT_EQ(update1.action, OrderUpdateAction::Remove);

    const auto& update2 = result.orderUpdates[1];
    EXPECT_EQ(update2.order->orderId, sellOrder2->orderId);
    EXPECT_EQ(update2.remainingQuantity, 0);
    EXPECT_EQ(update2.status, OrderStatus::Filled);
    EXPECT_EQ(update2.action, OrderUpdateAction::Remove);

    const auto& update3 = result.orderUpdates[2];
    EXPECT_EQ(update3.order->orderId, sellOrder3->orderId);
    EXPECT_EQ(update3.remainingQuantity, 20);
    EXPECT_EQ(update3.status, OrderStatus::PartiallyFilled);
    EXPECT_EQ(update3.action, OrderUpdateAction::Update);

    EXPECT_EQ(result.incomingOrderToInsert, nullptr);
}
