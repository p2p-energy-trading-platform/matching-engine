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

/*
 * Creates a GridTransferRule for same-zone trades.
 */
GridTransferRule createSameZoneRule(const GridZoneId& sellerZone, const GridZoneId& buyerZone) {
    return {.sellerGridZone = sellerZone,
            .buyerGridZone = buyerZone,
            .allowed = true,
            .gridFeePerKwh = 0,
            .version = 0};
}

/*
 * Determines if a candidate order is better than the current best order.
 */
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
    std::unordered_map<OrderId, Quantity> remainingQuantities;
    remainingQuantities.reserve(8);

    while (incomingBuy.remainingQuantity > Quantity{0}) {
        OrderPtr bestOrder = nullptr;
        GridTransferRule bestRule{};
        Price bestEffectivePrice{};
        Quantity bestAvailableQuantity{0};

        // Iterate through all grid zones to find the best matching SELL order.
        m_marketBook.withZoneOrderBooks([&](const auto& zoneOrderBooks) {
            for (const auto& [zoneId, zoneBook] : zoneOrderBooks) {
                const auto sellOrders = zoneBook.snapshotOrders(Side::Sell);

                if (sellOrders.empty()) {
                    continue;
                }

                for (const auto& restingOrder : sellOrders) {
                    GridTransferRule rule{};

                    if (zoneId == incomingBuy.gridZone) {
                        rule = createSameZoneRule(zoneId, incomingBuy.gridZone);
                    } else {
                        rule = m_gridTransferCache.resolve(zoneId, incomingBuy.gridZone);

                        if (!rule.allowed) {
                            continue;
                        }
                    }

                    Quantity availableQuantity = restingOrder->remainingQuantity;

                    if (const auto remainingIt = remainingQuantities.find(restingOrder->orderId);
                        remainingIt != remainingQuantities.end()) {
                        availableQuantity = remainingIt->second;
                    }

                    if (availableQuantity == Quantity{0}) {
                        continue;
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
                    bestAvailableQuantity = availableQuantity;
                }
            }
        });

        if (!bestOrder) {
            break;
        }
        const Quantity tradedQuantity =
            std::min(incomingBuy.remainingQuantity, bestAvailableQuantity);

        result.trades.push_back(m_tradeManager.createTrade(incomingBuy, *bestOrder, tradedQuantity,
                                                           bestOrder->price, bestRule));

        incomingBuy.remainingQuantity -= tradedQuantity;

        const Quantity remainingQuantity = bestAvailableQuantity - tradedQuantity;

        remainingQuantities[bestOrder->orderId] = remainingQuantity;

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
    std::unordered_map<OrderId, Quantity> remainingQuantities;
    remainingQuantities.reserve(8);

    while (incomingSell.remainingQuantity > Quantity{0}) {
        OrderPtr bestOrder = nullptr;
        GridTransferRule bestRule{};
        Price bestEffectiveBid{};
        Quantity bestAvailableQuantity{0};

        // Iterate through all grid zones to find the best matching BUY order.
        m_marketBook.withZoneOrderBooks([&](const auto& zoneOrderBooks) {
            for (const auto& [zoneId, zoneBook] : zoneOrderBooks) {
                const auto buyOrders = zoneBook.snapshotOrders(Side::Buy);

                if (buyOrders.empty()) {
                    continue;
                }

                for (const auto& restingOrder : buyOrders) {
                    GridTransferRule rule{};

                    if (zoneId == incomingSell.gridZone) {
                        rule = createSameZoneRule(incomingSell.gridZone, zoneId);
                    } else {
                        rule = m_gridTransferCache.resolve(incomingSell.gridZone, zoneId);

                        if (!rule.allowed) {
                            continue;
                        }
                    }

                    Quantity availableQuantity = restingOrder->remainingQuantity;

                    if (const auto remainingIt = remainingQuantities.find(restingOrder->orderId);
                        remainingIt != remainingQuantities.end()) {
                        availableQuantity = remainingIt->second;
                    }

                    if (availableQuantity == Quantity{0}) {
                        continue;
                    }

                    // Buyer's effective bid after paying grid fee.
                    const Price effectiveBid = restingOrder->price - rule.gridFeePerKwh;

                    if (effectiveBid < incomingSell.price) {
                        continue;
                    }

                    if (!isBetterCandidate(restingOrder, effectiveBid, zoneId, bestOrder,
                                           bestEffectiveBid, incomingSell.gridZone, true)) {
                        continue;
                    }

                    bestOrder = restingOrder;
                    bestRule = rule;
                    bestEffectiveBid = effectiveBid;
                    bestAvailableQuantity = availableQuantity;
                }
            }
        });

        if (!bestOrder) {
            break;
        }

        const Quantity tradedQuantity =
            std::min(incomingSell.remainingQuantity, bestAvailableQuantity);

        result.trades.push_back(m_tradeManager.createTrade(*bestOrder, incomingSell, tradedQuantity,
                                                           bestOrder->price, bestRule));

        incomingSell.remainingQuantity -= tradedQuantity;

        const Quantity remainingQuantity = bestAvailableQuantity - tradedQuantity;

        remainingQuantities[bestOrder->orderId] = remainingQuantity;

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