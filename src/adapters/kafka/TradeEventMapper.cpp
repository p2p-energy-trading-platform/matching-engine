#include "gridx/matching/adapters/kafka/TradeEventMapper.hpp"

#include <google/protobuf/timestamp.pb.h>

namespace gridx::matching::adapters::kafka {

gridx::trade::v1::TradeExecuted TradeEventMapper::toProtobuf(const Trade& trade) const {
    gridx::trade::v1::TradeExecuted event;

    // TODO: Replace with a proper Event ID generator.
    event.set_event_id("");

    event.set_trade_id(trade.tradeId);

    event.set_buy_order_id(trade.buyOrderId);
    event.set_sell_order_id(trade.sellOrderId);

    event.set_buyer_id(trade.buyerId);
    event.set_seller_id(trade.sellerId);

    event.set_buyer_grid_zone_id(trade.buyerGridZone);
    event.set_seller_grid_zone_id(trade.sellerGridZone);

    event.set_execution_price(trade.energyPrice);
    event.set_grid_fee_per_kwh(trade.gridFee);

    event.set_quantity(trade.quantity);

    event.set_grid_rule_version(trade.gridRuleVersion);

    auto* executedAt = event.mutable_executed_at();

    const auto duration = trade.timestamp.time_since_epoch();

    const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);

    const auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(duration - seconds);

    executedAt->set_seconds(seconds.count());
    executedAt->set_nanos(static_cast<int32_t>(nanos.count()));

    return event;
}

}  // namespace gridx::matching::adapters::kafka