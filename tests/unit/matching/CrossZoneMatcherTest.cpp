#include <chrono>

#include <gtest/gtest.h>

#include "gridx/matching/config/GridTransferCache.hpp"
#include "gridx/matching/domain/GridTransferRule.hpp"
#include "gridx/matching/matching/CrossZoneMatcher.hpp"
#include "gridx/matching/matching/TradeManager.hpp"
#include "gridx/matching/orderbook/MarketBook.hpp"

using namespace gridx::matching;
using namespace gridx::matching::matching;
using namespace gridx::matching::orderbook;

class CrossZoneMatcherTest : public ::testing::Test {
protected:
    CrossZoneMatcherTest()
        : marketId{
              std::chrono::system_clock::now(),
          }
        , marketBook(marketId)
        , matcher(marketBook, tradeManager, gridTransferCache) {}

    static constexpr GridZoneId kBuyerZone = 1;
    static constexpr GridZoneId kSellerZone = 2;

    MarketId marketId;

    MarketBook marketBook;

    TradeManager tradeManager;

    config::GridTransferCache gridTransferCache;

    CrossZoneMatcher matcher;
};



// Test that a BUY order fully matches a SELL order across different grid zones.
TEST_F(CrossZoneMatcherTest, BuyOrderFullyMatchesSellOrderAcrossZones) {
    GridTransferRule rule{};

    rule.sellerGridZone = kSellerZone;
    rule.buyerGridZone = kBuyerZone;
    rule.allowed = true;
    rule.gridFeePerKwh = 5;
    rule.version = 1;

    ASSERT_TRUE(gridTransferCache.upsert(rule));

    auto sellOrder = std::make_shared<Order>();

    sellOrder->orderId = 1;
    sellOrder->userId = 10;
    sellOrder->marketId = marketId;
    sellOrder->gridZone = kSellerZone;
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
    buyOrder.gridZone = kBuyerZone;
    buyOrder.side = Side::Buy;
    buyOrder.orderType = OrderType::Limit;
    buyOrder.status = OrderStatus::New;
    buyOrder.price = 105;
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

    EXPECT_EQ(trade.buyerGridZone, kBuyerZone);
    EXPECT_EQ(trade.sellerGridZone, kSellerZone);

    EXPECT_EQ(trade.energyPrice, 100);
    EXPECT_EQ(trade.quantity, 10);

    EXPECT_EQ(trade.gridFee, 5);
    EXPECT_EQ(trade.gridRuleVersion, 1);

    ASSERT_EQ(result.orderUpdates.size(), 1);

    const auto& update = result.orderUpdates.front();

    ASSERT_NE(update.order, nullptr);

    EXPECT_EQ(update.order->orderId, sellOrder->orderId);
    EXPECT_EQ(update.remainingQuantity, 0);
    EXPECT_EQ(update.status, OrderStatus::Filled);
    EXPECT_EQ(update.action, OrderUpdateAction::Remove);

    EXPECT_EQ(result.incomingOrderToInsert, nullptr);
}

// Test that a SELL order fully matches a BUY order across different grid zones.
TEST_F(CrossZoneMatcherTest, SellOrderFullyMatchesBuyOrderAcrossZones) {
    GridTransferRule rule{};

    rule.sellerGridZone = kSellerZone;
    rule.buyerGridZone = kBuyerZone;
    rule.allowed = true;
    rule.gridFeePerKwh = 5;
    rule.version = 1;

    ASSERT_TRUE(gridTransferCache.upsert(rule));

    auto buyOrder = std::make_shared<Order>();

    buyOrder->orderId = 1;
    buyOrder->userId = 10;
    buyOrder->marketId = marketId;
    buyOrder->gridZone = kBuyerZone;
    buyOrder->side = Side::Buy;
    buyOrder->orderType = OrderType::Limit;
    buyOrder->status = OrderStatus::New;
    buyOrder->price = 105;
    buyOrder->quantity = 10;
    buyOrder->remainingQuantity = 10;
    buyOrder->createdAt = std::chrono::system_clock::now();
    buyOrder->expiresAt = buyOrder->createdAt + std::chrono::hours(1);

    marketBook.addOrder(buyOrder);

    Order sellOrder{};

    sellOrder.orderId = 2;
    sellOrder.userId = 20;
    sellOrder.marketId = marketId;
    sellOrder.gridZone = kSellerZone;
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

    EXPECT_EQ(trade.buyerGridZone, kBuyerZone);
    EXPECT_EQ(trade.sellerGridZone, kSellerZone);

    EXPECT_EQ(trade.energyPrice, 105);
    EXPECT_EQ(trade.quantity, 10);

    EXPECT_EQ(trade.gridFee, 5);
    EXPECT_EQ(trade.gridRuleVersion, 1);

    ASSERT_EQ(result.orderUpdates.size(), 1);

    const auto& update = result.orderUpdates.front();

    ASSERT_NE(update.order, nullptr);

    EXPECT_EQ(update.order->orderId, buyOrder->orderId);
    EXPECT_EQ(update.remainingQuantity, 0);
    EXPECT_EQ(update.status, OrderStatus::Filled);
    EXPECT_EQ(update.action, OrderUpdateAction::Remove);

    EXPECT_EQ(result.incomingOrderToInsert, nullptr);
}


// Test that a BUY order partially matches a SELL order across different grid zones.
TEST_F(CrossZoneMatcherTest, BuyOrderPartiallyMatchesSellOrderAcrossZones) {
    GridTransferRule rule{};

    rule.sellerGridZone = kSellerZone;
    rule.buyerGridZone = kBuyerZone;
    rule.allowed = true;
    rule.gridFeePerKwh = 5;
    rule.version = 1;

    ASSERT_TRUE(gridTransferCache.upsert(rule));

    auto sellOrder = std::make_shared<Order>();

    sellOrder->orderId = 1;
    sellOrder->userId = 10;
    sellOrder->marketId = marketId;
    sellOrder->gridZone = kSellerZone;
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
    buyOrder.gridZone = kBuyerZone;
    buyOrder.side = Side::Buy;
    buyOrder.orderType = OrderType::Limit;
    buyOrder.status = OrderStatus::New;
    buyOrder.price = 105;
    buyOrder.quantity = 10;
    buyOrder.remainingQuantity = 10;
    buyOrder.createdAt = std::chrono::system_clock::now();
    buyOrder.expiresAt = buyOrder.createdAt + std::chrono::hours(1);

    const auto result = matcher.match(buyOrder);

    ASSERT_EQ(result.trades.size(), 1);

    const auto& trade = result.trades.front();

    EXPECT_EQ(trade.buyOrderId, buyOrder.orderId);
    EXPECT_EQ(trade.sellOrderId, sellOrder->orderId);

    EXPECT_EQ(trade.energyPrice, 100);
    EXPECT_EQ(trade.quantity, 10);
    EXPECT_EQ(trade.gridFee, 5);

    ASSERT_EQ(result.orderUpdates.size(), 1);

    const auto& update = result.orderUpdates.front();

    ASSERT_NE(update.order, nullptr);

    EXPECT_EQ(update.order->orderId, sellOrder->orderId);
    EXPECT_EQ(update.remainingQuantity, 10);
    EXPECT_EQ(update.status, OrderStatus::PartiallyFilled);
    EXPECT_EQ(update.action, OrderUpdateAction::Update);

    EXPECT_EQ(result.incomingOrderToInsert, nullptr);
}


