#include <chrono>

#include <gtest/gtest.h>

#include "gridx/matching/adapters/kafka/TradeEventMapper.hpp"

namespace gridx::matching::adapters::kafka {

class TradeEventMapperTest : public ::testing::Test {
protected:
    TradeEventMapper mapper;
};

TEST_F(TradeEventMapperTest, MapsTradeToProtobufEvent) {
    using namespace std::chrono;

    Trade trade{};

    trade.tradeId = 1001;

    trade.buyOrderId = 2001;
    trade.sellOrderId = 2002;

    trade.buyerId = 3001;
    trade.sellerId = 3002;

    trade.buyerGridZone = 10;
    trade.sellerGridZone = 20;

    trade.energyPrice = 125;
    trade.gridFee = 8;
    trade.quantity = 50;

    trade.gridRuleVersion = 4;

    trade.executedAt = std::chrono::system_clock::now();

    const auto event = mapper.toProtobuf(trade);

    EXPECT_TRUE(event.has_executed_at());

    EXPECT_EQ(event.trade_id(), trade.tradeId);

    EXPECT_EQ(event.buy_order_id(), trade.buyOrderId);
    EXPECT_EQ(event.sell_order_id(), trade.sellOrderId);

    EXPECT_EQ(event.buyer_id(), trade.buyerId);
    EXPECT_EQ(event.seller_id(), trade.sellerId);

    EXPECT_EQ(event.buyer_grid_zone_id(), trade.buyerGridZone);
    EXPECT_EQ(event.seller_grid_zone_id(), trade.sellerGridZone);

    EXPECT_EQ(event.execution_price(), trade.energyPrice);
    EXPECT_EQ(event.grid_fee_per_kwh(), trade.gridFee);

    EXPECT_EQ(event.quantity(), trade.quantity);

    EXPECT_EQ(event.grid_rule_version(), trade.gridRuleVersion);

    const auto duration = trade.executedAt.time_since_epoch();

    const auto seconds = duration_cast<std::chrono::seconds>(duration);

    const auto nanos = duration_cast<std::chrono::nanoseconds>(duration - seconds);

    EXPECT_EQ(event.executed_at().seconds(), seconds.count());
    EXPECT_EQ(event.executed_at().nanos(), nanos.count());
}

}  // namespace gridx::matching::adapters::kafka