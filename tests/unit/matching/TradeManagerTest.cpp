#include <gtest/gtest.h>

#include "gridx/matching/domain/GridTransferRule.hpp"
#include "gridx/matching/matching/TradeManager.hpp"
#include "support/TestSupport.hpp"

using namespace gridx::matching;
using namespace gridx::matching::matching;
using namespace gridx::matching::test_support;

namespace {}  // namespace

TEST(TradeManagerTest, CreateTradePopulatesAllAcceptanceFields) {
    const auto marketId = makeMarketId();

    const auto buyOrder = OrderBuilder{}
                              .withOrderId(1)
                              .withUserId(100)
                              .withMarketId(marketId)
                              .withGridZone(7)
                              .buy()
                              .withPrice(110)
                              .withQuantity(100)
                              .build();

    const auto sellOrder = OrderBuilder{}
                               .withOrderId(2)
                               .withUserId(200)
                               .withMarketId(marketId)
                               .withGridZone(8)
                               .sell()
                               .withPrice(95)
                               .withQuantity(50)
                               .build();

    GridTransferRule rule{};
    rule.gridFeePerKwh = 5;
    rule.version = 12;

    TradeManager tradeManager(std::make_unique<TradeIdGeneratorAdapter>(5000));

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
    const auto marketId = makeMarketId();

    const auto buyOrder = OrderBuilder{}
                              .withOrderId(10)
                              .withUserId(1000)
                              .withMarketId(marketId)
                              .withGridZone(1)
                              .buy()
                              .withPrice(120)
                              .withQuantity(100)
                              .build();

    const auto sellOrder = OrderBuilder{}
                               .withOrderId(20)
                               .withUserId(2000)
                               .withMarketId(marketId)
                               .withGridZone(2)
                               .sell()
                               .withPrice(90)
                               .withQuantity(100)
                               .build();

    const GridTransferRule rule{};

    TradeManager tradeManager(std::make_unique<TradeIdGeneratorAdapter>(42));

    const auto trade1 = tradeManager.createTrade(buyOrder, sellOrder, 30, 100, rule);
    const auto trade2 = tradeManager.createTrade(buyOrder, sellOrder, 20, 100, rule);
    const auto trade3 = tradeManager.createTrade(buyOrder, sellOrder, 10, 100, rule);

    EXPECT_EQ(trade1.tradeId, 42);
    EXPECT_EQ(trade2.tradeId, 43);
    EXPECT_EQ(trade3.tradeId, 44);
}