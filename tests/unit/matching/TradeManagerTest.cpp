#include <chrono>
#include <memory>

#include <gtest/gtest.h>

#include "gridx/matching/domain/GridTransferRule.hpp"
#include "gridx/matching/domain/Order.hpp"
#include "gridx/matching/matching/TradeIdGenerator.hpp"
#include "gridx/matching/matching/TradeManager.hpp"

using namespace gridx::matching;
using namespace gridx::matching::matching;

namespace {

class FixedSequenceTradeIdGenerator final : public ITradeIdGenerator {
public:
    explicit FixedSequenceTradeIdGenerator(TradeId start) : m_nextId(start) {}

    TradeId next() const override {
        return m_nextId++;
    }

private:
    mutable TradeId m_nextId;
};

struct OrderSpec {
    OrderId orderId;
    UserId userId;
    GridZoneId zone;
    Side side;
    Price price;
    Quantity quantity;
    MarketId marketId;
};

Order makeOrder(const OrderSpec& spec) {
    Order order{};
    order.orderId = spec.orderId;
    order.userId = spec.userId;
    order.marketId = spec.marketId;
    order.gridZone = spec.zone;
    order.side = spec.side;
    order.orderType = OrderType::Limit;
    order.status = OrderStatus::New;
    order.price = spec.price;
    order.quantity = spec.quantity;
    order.remainingQuantity = spec.quantity;
    order.createdAt = std::chrono::system_clock::now();
    order.expiresAt = order.createdAt + std::chrono::hours(1);
    return order;
}

}  // namespace

TEST(TradeManagerTest, CreateTradePopulatesAllAcceptanceFields) {
    const MarketId marketId{.deliverySlotStart = std::chrono::system_clock::now()};

    const auto buyOrder = makeOrder(
        OrderSpec{.orderId = 1,
                  .userId = 100,
                  .zone = 7,
                  .side = Side::Buy,
                  .price = 110,
                  .quantity = 100,
                  .marketId = marketId});

    const auto sellOrder = makeOrder(
        OrderSpec{.orderId = 2,
                  .userId = 200,
                  .zone = 8,
                  .side = Side::Sell,
                  .price = 95,
                  .quantity = 50,
                  .marketId = marketId});

    GridTransferRule rule{};
    rule.gridFeePerKwh = 5;
    rule.version = 12;

    TradeManager tradeManager(std::make_unique<FixedSequenceTradeIdGenerator>(5000));

    const auto trade = tradeManager.createTrade(buyOrder, sellOrder, 40, 100, rule);

    EXPECT_EQ(trade.tradeId, 5000);
    EXPECT_EQ(trade.buyOrderId, buyOrder.orderId);
    EXPECT_EQ(trade.sellOrderId, sellOrder.orderId);
    EXPECT_EQ(trade.buyerId, buyOrder.userId);
    EXPECT_EQ(trade.sellerId, sellOrder.userId);
    EXPECT_EQ(trade.buyerGridZone, buyOrder.gridZone);
    EXPECT_EQ(trade.sellerGridZone, sellOrder.gridZone);
    EXPECT_EQ(trade.deliverySlotStart, marketId.deliverySlotStart);
    EXPECT_EQ(trade.deliverySlotEnd, marketId.deliverySlotEnd());
    EXPECT_EQ(trade.energyPrice, 100);
    EXPECT_EQ(trade.gridFee, 5);
    EXPECT_EQ(trade.buyerTotalPrice, (100 + 5) * 40);
    EXPECT_EQ(trade.quantity, 40);
    EXPECT_EQ(trade.gridRuleVersion, 12);
    EXPECT_NE(trade.timestamp, Timestamp{});
}

TEST(TradeManagerTest, CreateTradeGeneratesUniqueIdsForMultipleTrades) {
    const MarketId marketId{.deliverySlotStart = std::chrono::system_clock::now()};

    const auto buyOrder = makeOrder(
        OrderSpec{.orderId = 10,
                  .userId = 1000,
                  .zone = 1,
                  .side = Side::Buy,
                  .price = 120,
                  .quantity = 100,
                  .marketId = marketId});

    const auto sellOrder = makeOrder(
        OrderSpec{.orderId = 20,
                  .userId = 2000,
                  .zone = 2,
                  .side = Side::Sell,
                  .price = 90,
                  .quantity = 100,
                  .marketId = marketId});

    const GridTransferRule rule{};

    TradeManager tradeManager(std::make_unique<FixedSequenceTradeIdGenerator>(42));

    const auto trade1 = tradeManager.createTrade(buyOrder, sellOrder, 30, 100, rule);
    const auto trade2 = tradeManager.createTrade(buyOrder, sellOrder, 20, 100, rule);
    const auto trade3 = tradeManager.createTrade(buyOrder, sellOrder, 10, 100, rule);

    EXPECT_EQ(trade1.tradeId, 42);
    EXPECT_EQ(trade2.tradeId, 43);
    EXPECT_EQ(trade3.tradeId, 44);
}