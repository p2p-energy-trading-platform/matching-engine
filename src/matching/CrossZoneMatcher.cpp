#include <algorithm>
#include <utility>

#include "gridx/matching/matching/CrossZoneMatcher.hpp"

#include "gridx/matching/config/GridTransferCache.hpp"
#include "gridx/matching/domain/GridTransferRule.hpp"
#include "gridx/matching/matching/TradeManager.hpp"
#include "gridx/matching/orderbook/MarketBook.hpp"
#include "gridx/matching/orderbook/ZoneOrderBook.hpp"

namespace gridx::matching::matching {

CrossZoneMatcher::CrossZoneMatcher(orderbook::MarketBook& marketBook, TradeManager& tradeManager,
                                   const config::GridTransferCache& gridTransferCache)
    : m_marketBook(marketBook)
    , m_tradeManager(tradeManager)
    , m_gridTransferCache(gridTransferCache) {}

MatchingResult CrossZoneMatcher::match(Order incomingOrder) {
    if (incomingOrder.side == Side::Buy) {
        return matchBuy(std::move(incomingOrder));
    }

    return matchSell(std::move(incomingOrder));
}


GridTransferRule createSameZoneRule(const GridZoneId& sellerZone, const GridZoneId& buyerZone) {
    return {.sellerGridZone = sellerZone,
            .buyerGridZone = buyerZone,
            .allowed = true,
            .gridFeePerKwh = 0,
            .version = 0};
}

bool CrossZoneMatcher::isBetterCandidate(const OrderPtr& candidateOrder,
                                         Price candidateEffectivePrice,
                                         const GridZoneId& candidateZone,
                                         const OrderPtr& currentBestOrder,
                                         Price currentBestEffectivePrice,
                                         const GridZoneId& incomingZone, bool higherPriceWins) {
    if (!currentBestOrder) {
        return true;
    }

    // Better effective price.
    if (candidateEffectivePrice != currentBestEffectivePrice) {
        return higherPriceWins ? candidateEffectivePrice > currentBestEffectivePrice
                               : candidateEffectivePrice < currentBestEffectivePrice;
    }

    // Earlier order wins.
    if (candidateOrder->createdAt != currentBestOrder->createdAt) {
        return candidateOrder->createdAt < currentBestOrder->createdAt;
    }

    // Prefer same-zone when everything else is equal.
    const bool candidateSameZone = candidateZone == incomingZone;

    const bool currentSameZone = currentBestOrder->gridZone == incomingZone;

    return candidateSameZone && !currentSameZone;
}

MatchingResult CrossZoneMatcher::matchBuy(Order incomingBuy) const {
    MatchingResult result;

    while (incomingBuy.remainingQuantity > Quantity{0}) {
        OrderPtr bestOrder = nullptr;
        GridTransferRule bestRule{};
        Price bestEffectivePrice{};

        for (const auto& [zoneId, zoneBook] : m_marketBook.zoneOrderBooks()) {
            const auto& sellBook = zoneBook.sellBook();

            if (sellBook.empty()) {
                continue;
            }

            OrderPtr restingOrder = sellBook.bestOrder();

            GridTransferRule rule{};
            if (zoneId == incomingBuy.gridZone) {
                rule = createSameZoneRule(zoneId, incomingBuy.gridZone);
            } else {
                rule = m_gridTransferCache.resolve(zoneId, incomingBuy.gridZone);

                if (!rule.allowed) {
                    continue;
                }
            }

            const Price effectiveAsk = restingOrder->price + rule.gridFeePerKwh;

            if (effectiveAsk > incomingBuy.price) {
                continue;
            }

            if (!isBetterCandidate(restingOrder, effectiveAsk, zoneId, bestOrder,
                                   bestEffectivePrice, incomingBuy.gridZone, false)) {
                continue;
            }

            bestOrder = restingOrder;
            bestRule = rule;
            bestEffectivePrice = effectiveAsk;
        }

        if (!bestOrder) {
            break;
        }

        const Quantity tradedQuantity =
            std::min(incomingBuy.remainingQuantity, bestOrder->remainingQuantity);

        result.trades.push_back(m_tradeManager.createTrade(incomingBuy, *bestOrder, tradedQuantity,
                                                           bestOrder->price, bestRule));

        incomingBuy.remainingQuantity -= tradedQuantity;

        const Quantity remainingQuantity = bestOrder->remainingQuantity - tradedQuantity;

        const OrderStatus status =
            remainingQuantity == 0 ? OrderStatus::Filled : OrderStatus::PartiallyFilled;

        result.orderUpdates.push_back(OrderUpdate{.order = bestOrder,
                                                  .remainingQuantity = remainingQuantity,
                                                  .status = status,
                                                  .action = remainingQuantity == 0
                                                                ? OrderUpdateAction::Remove
                                                                : OrderUpdateAction::Update});
    }

    if (incomingBuy.remainingQuantity > Quantity{0}) {
        incomingBuy.status = incomingBuy.remainingQuantity == incomingBuy.quantity
                                 ? OrderStatus::New
                                 : OrderStatus::PartiallyFilled;

        result.incomingOrderToInsert = std::make_shared<Order>(std::move(incomingBuy));
    }

    return result;
}

MatchingResult CrossZoneMatcher::matchSell(Order incomingSell) const {
    MatchingResult result;

    while (incomingSell.remainingQuantity > Quantity{0}) {
        OrderPtr bestOrder = nullptr;
        GridTransferRule bestRule{};
        Price bestEffectiveBid{};

        for (const auto& [zoneId, zoneBook] : m_marketBook.zoneOrderBooks()) {
            const auto& buyBook = zoneBook.buyBook();

            if (buyBook.empty()) {
                continue;
            }

            OrderPtr restingOrder = buyBook.bestOrder();

            GridTransferRule rule{};

            if (zoneId == incomingSell.gridZone) {
                rule = createSameZoneRule(incomingSell.gridZone, zoneId);
            } else {
                rule = m_gridTransferCache.resolve(incomingSell.gridZone, zoneId);

                if (!rule.allowed) {
                    continue;
                }
            }

            // Buyer's effective bid after paying grid fee.
            const Price effectiveBid = restingOrder->price - rule.gridFeePerKwh;

            if (effectiveBid < incomingSell.price) {
                continue;
            }

            if (!isBetterCandidate(restingOrder, effectiveBid, zoneId, bestOrder, bestEffectiveBid,
                                   incomingSell.gridZone, true)) {
                continue;
            }

            bestOrder = restingOrder;
            bestRule = rule;
            bestEffectiveBid = effectiveBid;
        }

        if (!bestOrder) {
            break;
        }

        const Quantity tradedQuantity =
            std::min(incomingSell.remainingQuantity, bestOrder->remainingQuantity);

        result.trades.push_back(m_tradeManager.createTrade(*bestOrder, incomingSell, tradedQuantity,
                                                           bestOrder->price, bestRule));

        incomingSell.remainingQuantity -= tradedQuantity;

        const Quantity remainingQuantity = bestOrder->remainingQuantity - tradedQuantity;

        const OrderStatus status =
            remainingQuantity == 0 ? OrderStatus::Filled : OrderStatus::PartiallyFilled;

        result.orderUpdates.push_back(OrderUpdate{.order = bestOrder,
                                                  .remainingQuantity = remainingQuantity,
                                                  .status = status,
                                                  .action = remainingQuantity == 0
                                                                ? OrderUpdateAction::Remove
                                                                : OrderUpdateAction::Update});
    }

    if (incomingSell.remainingQuantity > Quantity{0}) {
        incomingSell.status = incomingSell.remainingQuantity == incomingSell.quantity
                                  ? OrderStatus::New
                                  : OrderStatus::PartiallyFilled;

        result.incomingOrderToInsert = std::make_shared<Order>(std::move(incomingSell));
    }

    return result;
}
}  // namespace gridx::matching::matching