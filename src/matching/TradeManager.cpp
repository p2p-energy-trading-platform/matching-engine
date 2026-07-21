#include "gridx/matching/matching/TradeManager.hpp"

#include <chrono>

namespace gridx::matching::matching
{

Trade TradeManager::createTrade(
    const Order& buyOrder,
    const Order& sellOrder,
    Quantity tradedQuantity,
    Price executionPrice,
    GridFee gridFee) const
{
    Trade trade{};

    // TODO: Replace with Trade ID generator.
    trade.tradeId = 0;

    trade.buyOrderId = buyOrder.orderId;
    trade.sellOrderId = sellOrder.orderId;

    trade.buyerId = buyOrder.userId;
    trade.sellerId = sellOrder.userId;

    trade.buyerGridZone = buyOrder.gridZone;
    trade.sellerGridZone = sellOrder.gridZone;

    trade.energyPrice = executionPrice;
    trade.gridFee = gridFee;
    trade.quantity = tradedQuantity;

    // Same-zone trades currently don't require grid transfer rules.
    // TODO: Populate from Grid Rule Service once cross-zone matching is implemented.
    trade.gridRuleVersion = 0;

    // TODO: Replace with injectable clock if deterministic timestamps are required.
    trade.executedAt = std::chrono::system_clock::now();

    return trade;
}

} // namespace gridx::matching::matching