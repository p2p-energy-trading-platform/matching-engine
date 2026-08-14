#include <chrono>

#include <gtest/gtest.h>

#include "gridx/matching/config/GridTransferCache.hpp"
#include "gridx/matching/domain/GridTransferRule.hpp"
#include "gridx/matching/matching/CrossZoneMatcher.hpp"
#include "gridx/matching/matching/TradeManager.hpp"
#include "gridx/matching/orderbook/MarketBook.hpp"
#include "support/TestSupport.hpp"

using namespace gridx::matching;
using namespace gridx::matching::matching;
using namespace gridx::matching::orderbook;
using namespace gridx::matching::test_support;

class CrossZoneMatcherTest : public ::testing::Test {
protected:
    CrossZoneMatcherTest()
        : marketId{
              std::chrono::system_clock::now(),
          }
        , marketBook(marketId)
        , matcher(marketBook, tradeManager, gridTransferCache) {}

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

    Order makeOrderValue(OrderId orderId, UserId userId, GridZoneId gridZone, Side side,
                         Price price, Quantity quantity,
                         const Timestamp& createdAt = fixedTimestamp()) const {
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
            .build();
    }

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

    auto sellOrder =
        makeOrder(1, 10, kSellerZone, Side::Sell, 100, 10, std::chrono::system_clock::now());

    marketBook.addOrder(sellOrder);

    Order buyOrder =
        makeOrderValue(2, 20, kBuyerZone, Side::Buy, 105, 10, std::chrono::system_clock::now());

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

    auto buyOrder =
        makeOrder(1, 10, kBuyerZone, Side::Buy, 105, 10, std::chrono::system_clock::now());

    marketBook.addOrder(buyOrder);

    Order sellOrder =
        makeOrderValue(2, 20, kSellerZone, Side::Sell, 100, 10, std::chrono::system_clock::now());

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

    auto sellOrder =
        makeOrder(1, 10, kSellerZone, Side::Sell, 100, 20, std::chrono::system_clock::now());

    marketBook.addOrder(sellOrder);

    Order buyOrder =
        makeOrderValue(2, 20, kBuyerZone, Side::Buy, 105, 10, std::chrono::system_clock::now());

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

// Test that a BUY order matches multiple SELL orders across different grid zones.
TEST_F(CrossZoneMatcherTest, BuyOrderMatchesMultipleSellOrdersAcrossZones) {
    GridTransferRule rule1{};

    rule1.sellerGridZone = kSellerZone;
    rule1.buyerGridZone = kBuyerZone;
    rule1.allowed = true;
    rule1.gridFeePerKwh = 5;
    rule1.version = 1;

    ASSERT_TRUE(gridTransferCache.upsert(rule1));

    GridTransferRule rule2{};

    rule2.sellerGridZone = 3;
    rule2.buyerGridZone = kBuyerZone;
    rule2.allowed = true;
    rule2.gridFeePerKwh = 2;
    rule2.version = 1;

    ASSERT_TRUE(gridTransferCache.upsert(rule2));

    auto sellOrder1 = std::make_shared<Order>();

    sellOrder1->orderId = 1;
    sellOrder1->userId = 10;
    sellOrder1->marketId = marketId;
    sellOrder1->gridZone = kSellerZone;
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
    sellOrder2->gridZone = 3;
    sellOrder2->side = Side::Sell;
    sellOrder2->orderType = OrderType::Limit;
    sellOrder2->status = OrderStatus::New;
    sellOrder2->price = 101;
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
    buyOrder.gridZone = kBuyerZone;
    buyOrder.side = Side::Buy;
    buyOrder.orderType = OrderType::Limit;
    buyOrder.status = OrderStatus::New;
    buyOrder.price = 105;
    buyOrder.quantity = 15;
    buyOrder.remainingQuantity = 15;
    buyOrder.createdAt = std::chrono::system_clock::now();
    buyOrder.expiresAt = buyOrder.createdAt + std::chrono::hours(1);

    const auto result = matcher.match(buyOrder);

    ASSERT_EQ(result.trades.size(), 2);

    EXPECT_EQ(result.trades[0].sellOrderId, sellOrder2->orderId);
    EXPECT_EQ(result.trades[0].quantity, 10);
    EXPECT_EQ(result.trades[0].gridFee, 2);

    EXPECT_EQ(result.trades[1].sellOrderId, sellOrder1->orderId);
    EXPECT_EQ(result.trades[1].quantity, 5);
    EXPECT_EQ(result.trades[1].gridFee, 5);

    ASSERT_EQ(result.orderUpdates.size(), 2);

    const auto& update1 = result.orderUpdates.front();

    EXPECT_EQ(update1.order->orderId, sellOrder2->orderId);
    EXPECT_EQ(update1.remainingQuantity, 0);
    EXPECT_EQ(update1.status, OrderStatus::Filled);
    EXPECT_EQ(update1.action, OrderUpdateAction::Remove);

    const auto& update2 = result.orderUpdates.back();

    EXPECT_EQ(update2.order->orderId, sellOrder1->orderId);
    EXPECT_EQ(update2.remainingQuantity, 0);
    EXPECT_EQ(update2.status, OrderStatus::Filled);
    EXPECT_EQ(update2.action, OrderUpdateAction::Remove);

    EXPECT_EQ(result.incomingOrderToInsert, nullptr);
}

// Test that a BUY order does not match a SELL order when cross-zone transfer is not allowed.
TEST_F(CrossZoneMatcherTest, BuyOrderDoesNotMatchWhenTransferNotAllowed) {
    GridTransferRule rule{};

    rule.sellerGridZone = kSellerZone;
    rule.buyerGridZone = kBuyerZone;
    rule.allowed = false;
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

    EXPECT_TRUE(result.trades.empty());

    EXPECT_TRUE(result.orderUpdates.empty());

    ASSERT_NE(result.incomingOrderToInsert, nullptr);

    EXPECT_EQ(result.incomingOrderToInsert->orderId, buyOrder.orderId);
    EXPECT_EQ(result.incomingOrderToInsert->remainingQuantity, 10);
    EXPECT_EQ(result.incomingOrderToInsert->price, 105);
    EXPECT_EQ(result.incomingOrderToInsert->side, Side::Buy);
}

// Test that a BUY order matches the SELL order with the lowest effective ask across zones.
TEST_F(CrossZoneMatcherTest, BuyOrderChoosesLowestEffectiveAsk) {
    GridTransferRule rule1{};

    rule1.sellerGridZone = kSellerZone;
    rule1.buyerGridZone = kBuyerZone;
    rule1.allowed = true;
    rule1.gridFeePerKwh = 5;
    rule1.version = 1;

    ASSERT_TRUE(gridTransferCache.upsert(rule1));

    GridTransferRule rule2{};

    rule2.sellerGridZone = 3;
    rule2.buyerGridZone = kBuyerZone;
    rule2.allowed = true;
    rule2.gridFeePerKwh = 15;
    rule2.version = 1;

    ASSERT_TRUE(gridTransferCache.upsert(rule2));

    auto sellOrder1 = std::make_shared<Order>();

    sellOrder1->orderId = 1;
    sellOrder1->userId = 10;
    sellOrder1->marketId = marketId;
    sellOrder1->gridZone = kSellerZone;
    sellOrder1->side = Side::Sell;
    sellOrder1->orderType = OrderType::Limit;
    sellOrder1->status = OrderStatus::New;
    sellOrder1->price = 100;
    sellOrder1->quantity = 10;
    sellOrder1->remainingQuantity = 10;
    sellOrder1->createdAt = std::chrono::system_clock::now();
    sellOrder1->expiresAt = sellOrder1->createdAt + std::chrono::hours(1);

    auto sellOrder2 = std::make_shared<Order>();

    sellOrder2->orderId = 2;
    sellOrder2->userId = 11;
    sellOrder2->marketId = marketId;
    sellOrder2->gridZone = 3;
    sellOrder2->side = Side::Sell;
    sellOrder2->orderType = OrderType::Limit;
    sellOrder2->status = OrderStatus::New;
    sellOrder2->price = 95;
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
    buyOrder.gridZone = kBuyerZone;
    buyOrder.side = Side::Buy;
    buyOrder.orderType = OrderType::Limit;
    buyOrder.status = OrderStatus::New;
    buyOrder.price = 110;
    buyOrder.quantity = 10;
    buyOrder.remainingQuantity = 10;
    buyOrder.createdAt = std::chrono::system_clock::now();
    buyOrder.expiresAt = buyOrder.createdAt + std::chrono::hours(1);

    const auto result = matcher.match(buyOrder);

    ASSERT_EQ(result.trades.size(), 1);

    const auto& trade = result.trades.front();

    // Zone 2 should win because:
    // Zone 2: 100 + 5 = 105
    // Zone 3: 95 + 15 = 110
    EXPECT_EQ(trade.sellOrderId, sellOrder1->orderId);
    EXPECT_EQ(trade.sellerGridZone, kSellerZone);
    EXPECT_EQ(trade.energyPrice, 100);
    EXPECT_EQ(trade.gridFee, 5);

    ASSERT_EQ(result.orderUpdates.size(), 1);

    EXPECT_EQ(result.orderUpdates.front().order->orderId, sellOrder1->orderId);
}

// Test that a SELL order matches the BUY order with the highest effective bid across zones.
TEST_F(CrossZoneMatcherTest, SellOrderChoosesHighestEffectiveBid) {
    GridTransferRule rule1{};

    rule1.sellerGridZone = kSellerZone;
    rule1.buyerGridZone = kBuyerZone;
    rule1.allowed = true;
    rule1.gridFeePerKwh = 5;
    rule1.version = 1;

    ASSERT_TRUE(gridTransferCache.upsert(rule1));

    GridTransferRule rule2{};

    rule2.sellerGridZone = kSellerZone;
    rule2.buyerGridZone = 3;
    rule2.allowed = true;
    rule2.gridFeePerKwh = 15;
    rule2.version = 1;

    ASSERT_TRUE(gridTransferCache.upsert(rule2));

    auto buyOrder1 = std::make_shared<Order>();

    buyOrder1->orderId = 1;
    buyOrder1->userId = 10;
    buyOrder1->marketId = marketId;
    buyOrder1->gridZone = kBuyerZone;
    buyOrder1->side = Side::Buy;
    buyOrder1->orderType = OrderType::Limit;
    buyOrder1->status = OrderStatus::New;
    buyOrder1->price = 105;
    buyOrder1->quantity = 10;
    buyOrder1->remainingQuantity = 10;
    buyOrder1->createdAt = std::chrono::system_clock::now();
    buyOrder1->expiresAt = buyOrder1->createdAt + std::chrono::hours(1);

    auto buyOrder2 = std::make_shared<Order>();

    buyOrder2->orderId = 2;
    buyOrder2->userId = 11;
    buyOrder2->marketId = marketId;
    buyOrder2->gridZone = 3;
    buyOrder2->side = Side::Buy;
    buyOrder2->orderType = OrderType::Limit;
    buyOrder2->status = OrderStatus::New;
    buyOrder2->price = 120;
    buyOrder2->quantity = 10;
    buyOrder2->remainingQuantity = 10;
    buyOrder2->createdAt = std::chrono::system_clock::now();
    buyOrder2->expiresAt = buyOrder2->createdAt + std::chrono::hours(1);

    marketBook.addOrder(buyOrder1);
    marketBook.addOrder(buyOrder2);

    Order sellOrder{};

    sellOrder.orderId = 3;
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

    // Zone 1: 105 - 5 = 100
    // Zone 3: 120 - 15 = 105 (better)
    EXPECT_EQ(trade.buyOrderId, buyOrder2->orderId);
    EXPECT_EQ(trade.buyerGridZone, 3);
    EXPECT_EQ(trade.energyPrice, 120);
    EXPECT_EQ(trade.gridFee, 15);

    ASSERT_EQ(result.orderUpdates.size(), 1);

    const auto& update = result.orderUpdates.front();

    ASSERT_NE(update.order, nullptr);
    EXPECT_EQ(update.order->orderId, buyOrder2->orderId);
    EXPECT_EQ(update.remainingQuantity, 0);
    EXPECT_EQ(update.status, OrderStatus::Filled);
    EXPECT_EQ(update.action, OrderUpdateAction::Remove);

    EXPECT_EQ(result.incomingOrderToInsert, nullptr);
}

// Test that the earlier order is matched when effective prices are equal.
TEST_F(CrossZoneMatcherTest, EarlierOrderWinsWhenEffectivePriceEqual) {
    GridTransferRule rule1{};

    rule1.sellerGridZone = kSellerZone;
    rule1.buyerGridZone = kBuyerZone;
    rule1.allowed = true;
    rule1.gridFeePerKwh = 5;
    rule1.version = 1;

    ASSERT_TRUE(gridTransferCache.upsert(rule1));

    GridTransferRule rule2{};

    rule2.sellerGridZone = 3;
    rule2.buyerGridZone = kBuyerZone;
    rule2.allowed = true;
    rule2.gridFeePerKwh = 0;
    rule2.version = 1;

    ASSERT_TRUE(gridTransferCache.upsert(rule2));

    const auto earlier = std::chrono::system_clock::now();
    const auto later = earlier + std::chrono::seconds(10);

    auto sellOrder1 = std::make_shared<Order>();

    sellOrder1->orderId = 1;
    sellOrder1->userId = 10;
    sellOrder1->marketId = marketId;
    sellOrder1->gridZone = kSellerZone;
    sellOrder1->side = Side::Sell;
    sellOrder1->orderType = OrderType::Limit;
    sellOrder1->status = OrderStatus::New;
    sellOrder1->price = 100;
    sellOrder1->quantity = 10;
    sellOrder1->remainingQuantity = 10;
    sellOrder1->createdAt = later;
    sellOrder1->expiresAt = later + std::chrono::hours(1);

    auto sellOrder2 = std::make_shared<Order>();

    sellOrder2->orderId = 2;
    sellOrder2->userId = 11;
    sellOrder2->marketId = marketId;
    sellOrder2->gridZone = 3;
    sellOrder2->side = Side::Sell;
    sellOrder2->orderType = OrderType::Limit;
    sellOrder2->status = OrderStatus::New;
    sellOrder2->price = 105;
    sellOrder2->quantity = 10;
    sellOrder2->remainingQuantity = 10;
    sellOrder2->createdAt = earlier;
    sellOrder2->expiresAt = earlier + std::chrono::hours(1);

    marketBook.addOrder(sellOrder1);
    marketBook.addOrder(sellOrder2);

    Order buyOrder{};

    buyOrder.orderId = 3;
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

    // Effective ask:
    // Zone 2: 100 + 5 = 105
    // Zone 3: 105 + 0 = 105
    // Earlier order should win (Zone 3).
    EXPECT_EQ(trade.sellOrderId, sellOrder2->orderId);
    EXPECT_EQ(trade.sellerGridZone, 3);
}

// Test that a same-zone order is preferred when effective price and timestamp are equal.
TEST_F(CrossZoneMatcherTest, SameZonePreferredWhenEffectivePriceAndTimestampEqual) {
    GridTransferRule rule{};

    rule.sellerGridZone = kSellerZone;
    rule.buyerGridZone = kBuyerZone;
    rule.allowed = true;
    rule.gridFeePerKwh = 5;
    rule.version = 1;

    ASSERT_TRUE(gridTransferCache.upsert(rule));

    const auto createdTime = std::chrono::system_clock::now();

    auto sameZoneSellOrder = std::make_shared<Order>();

    sameZoneSellOrder->orderId = 1;
    sameZoneSellOrder->userId = 10;
    sameZoneSellOrder->marketId = marketId;
    sameZoneSellOrder->gridZone = kBuyerZone;
    sameZoneSellOrder->side = Side::Sell;
    sameZoneSellOrder->orderType = OrderType::Limit;
    sameZoneSellOrder->status = OrderStatus::New;
    sameZoneSellOrder->price = 105;
    sameZoneSellOrder->quantity = 10;
    sameZoneSellOrder->remainingQuantity = 10;
    sameZoneSellOrder->createdAt = createdTime;
    sameZoneSellOrder->expiresAt = createdTime + std::chrono::hours(1);

    auto crossZoneSellOrder = std::make_shared<Order>();

    crossZoneSellOrder->orderId = 2;
    crossZoneSellOrder->userId = 11;
    crossZoneSellOrder->marketId = marketId;
    crossZoneSellOrder->gridZone = kSellerZone;
    crossZoneSellOrder->side = Side::Sell;
    crossZoneSellOrder->orderType = OrderType::Limit;
    crossZoneSellOrder->status = OrderStatus::New;
    crossZoneSellOrder->price = 100;
    crossZoneSellOrder->quantity = 10;
    crossZoneSellOrder->remainingQuantity = 10;
    crossZoneSellOrder->createdAt = createdTime;
    crossZoneSellOrder->expiresAt = createdTime + std::chrono::hours(1);

    marketBook.addOrder(sameZoneSellOrder);
    marketBook.addOrder(crossZoneSellOrder);

    Order buyOrder{};

    buyOrder.orderId = 3;
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

    // Effective ask:
    // Same-zone : 105 + 0 = 105
    // Cross-zone: 100 + 5 = 105
    // Same timestamp -> same-zone should win.
    EXPECT_EQ(trade.sellOrderId, sameZoneSellOrder->orderId);
    EXPECT_EQ(trade.sellerGridZone, kBuyerZone);
    EXPECT_EQ(trade.gridFee, 0);
}

// Test that a trade between orders in the same grid zone has a grid fee of zero.
TEST_F(CrossZoneMatcherTest, SameZoneTradeHasZeroGridFee) {
    auto sellOrder = std::make_shared<Order>();

    sellOrder->orderId = 1;
    sellOrder->userId = 10;
    sellOrder->marketId = marketId;
    sellOrder->gridZone = kBuyerZone;
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

    EXPECT_EQ(trade.buyerGridZone, kBuyerZone);
    EXPECT_EQ(trade.sellerGridZone, kBuyerZone);

    EXPECT_EQ(trade.energyPrice, 100);
    EXPECT_EQ(trade.quantity, 10);

    EXPECT_EQ(trade.gridFee, 0);
    EXPECT_EQ(trade.gridRuleVersion, 0);

    ASSERT_EQ(result.orderUpdates.size(), 1);

    const auto& update = result.orderUpdates.front();

    ASSERT_NE(update.order, nullptr);
    EXPECT_EQ(update.order->orderId, sellOrder->orderId);
    EXPECT_EQ(update.remainingQuantity, 0);
    EXPECT_EQ(update.status, OrderStatus::Filled);
    EXPECT_EQ(update.action, OrderUpdateAction::Remove);

    EXPECT_EQ(result.incomingOrderToInsert, nullptr);
}
