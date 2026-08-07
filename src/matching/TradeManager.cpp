#include "gridx/matching/matching/TradeManager.hpp"

#include <chrono>
#include <utility>

namespace gridx::matching::matching {

namespace {

Price calculateBuyerTotalPrice(Price energyPrice, GridFee gridFee, Quantity quantity) {
    return (energyPrice + gridFee) * quantity;
}

}  // namespace

TradeManager::TradeManager(std::unique_ptr<ITradeIdGenerator> tradeIdGenerator)
    : m_tradeIdGenerator(tradeIdGenerator ? std::move(tradeIdGenerator)
                                          : std::make_unique<AtomicTradeIdGenerator>()) {}

Trade TradeManager::createTrade(const Order& buyOrder, const Order& sellOrder,
                                Quantity tradedQuantity, Price executionPrice,
                                const GridTransferRule& rule) const {
    Trade trade{};

    trade.tradeId = m_tradeIdGenerator->next();

    trade.buyOrderId = buyOrder.orderId;
    trade.sellOrderId = sellOrder.orderId;

    trade.buyerId = buyOrder.userId;
    trade.sellerId = sellOrder.userId;

    trade.buyerGridZone = buyOrder.gridZone;
    trade.sellerGridZone = sellOrder.gridZone;

    trade.deliverySlotStart = buyOrder.marketId.deliverySlotStart;
    trade.deliverySlotEnd = buyOrder.marketId.deliverySlotEnd();

    trade.energyPrice = executionPrice;
    trade.gridFee = rule.gridFeePerKwh;
    trade.buyerTotalPrice =
        calculateBuyerTotalPrice(executionPrice, rule.gridFeePerKwh, tradedQuantity);

    trade.quantity = tradedQuantity;

    // Grid transfer rule version applied when the trade was executed.
    trade.gridRuleVersion = rule.version;

    // TODO: Replace with injectable clock if deterministic timestamps are required.
    trade.timestamp = std::chrono::system_clock::now();

    return trade;
}

}  // namespace gridx::matching::matching