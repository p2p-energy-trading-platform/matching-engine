#include <stdexcept>
#include <utility>

#include "gridx/matching/matching/SameZoneMatcher.hpp"
#include "gridx/matching/matching/TradeManager.hpp"
#include "gridx/matching/orderbook/MarketBook.hpp"
#include "gridx/matching/orderbook/ZoneOrderBook.hpp"

namespace gridx::matching::matching {

constexpr GridTransferRule kSameZoneRule{.allowed = true, .gridFeePerKwh = 0, .version = 0};

SameZoneMatcher::SameZoneMatcher(orderbook::MarketBook& marketBook, TradeManager& tradeManager)
    : m_marketBook(marketBook), m_tradeManager(tradeManager) {}

// Matches an incoming order against opposite-side orders within the same grid zone.
MatchingResult SameZoneMatcher::match(Order incomingOrder) {
    auto& zoneBook = m_marketBook.zoneOrderBook(incomingOrder.gridZone);

    if (incomingOrder.side == Side::Buy) {
        return matchBuy(incomingOrder, zoneBook);
    }

    return matchSell(incomingOrder, zoneBook);
}

// Matches an incoming BUY order against the SELL book.
MatchingResult SameZoneMatcher::matchBuy(Order incomingBuy,
                                         const orderbook::ZoneOrderBook& zoneBook) const {
    MatchingResult result;

    const auto& sellBook = zoneBook.sellBook();

    for (auto it = sellBook.ordersBegin();
         it != sellBook.ordersEnd() && incomingBuy.remainingQuantity > 0; ++it) {
        const auto& restingOrder = *it;

        if (restingOrder->price > incomingBuy.price) {
            break;
        }

        const Quantity tradedQuantity =
            std::min(incomingBuy.remainingQuantity, restingOrder->remainingQuantity);

        result.trades.push_back(m_tradeManager.createTrade(
            incomingBuy, *restingOrder, tradedQuantity, restingOrder->price, kSameZoneRule));

        incomingBuy.remainingQuantity -= tradedQuantity;

        const Quantity remainingQuantity = restingOrder->remainingQuantity - tradedQuantity;

        const OrderStatus status =
            remainingQuantity == 0 ? OrderStatus::Filled : OrderStatus::PartiallyFilled;

        result.orderUpdates.push_back(OrderUpdate{.order = restingOrder,
                                                  .remainingQuantity = remainingQuantity,
                                                  .status = status,
                                                  .action = remainingQuantity == 0
                                                                ? OrderUpdateAction::Remove
                                                                : OrderUpdateAction::Update});
    }

    if (incomingBuy.remainingQuantity > 0) {
        incomingBuy.status = incomingBuy.remainingQuantity == incomingBuy.quantity
                                 ? OrderStatus::New
                                 : OrderStatus::PartiallyFilled;

        result.incomingOrderToInsert = std::make_shared<Order>(std::move(incomingBuy));
    }

    return result;
}

// Matches an incoming SELL order against the BUY book.
MatchingResult SameZoneMatcher::matchSell(Order incomingSell,
                                          const orderbook::ZoneOrderBook& zoneBook) const {
    MatchingResult result;

    const auto& buyBook = zoneBook.buyBook();

    for (auto it = buyBook.ordersBegin();
         it != buyBook.ordersEnd() && incomingSell.remainingQuantity > 0; ++it) {
        const auto& restingOrder = *it;

        if (restingOrder->price < incomingSell.price) {
            break;
        }

        const Quantity tradedQuantity =
            std::min(incomingSell.remainingQuantity, restingOrder->remainingQuantity);

        result.trades.push_back(m_tradeManager.createTrade(
            *restingOrder, incomingSell, tradedQuantity, restingOrder->price, kSameZoneRule));

        incomingSell.remainingQuantity -= tradedQuantity;

        const Quantity remainingQuantity = restingOrder->remainingQuantity - tradedQuantity;

        const OrderStatus status =
            remainingQuantity == 0 ? OrderStatus::Filled : OrderStatus::PartiallyFilled;

        result.orderUpdates.push_back(OrderUpdate{.order = restingOrder,
                                                  .remainingQuantity = remainingQuantity,
                                                  .status = status,
                                                  .action = remainingQuantity == 0
                                                                ? OrderUpdateAction::Remove
                                                                : OrderUpdateAction::Update});
    }

    if (incomingSell.remainingQuantity > 0) {
        incomingSell.status = incomingSell.remainingQuantity == incomingSell.quantity
                                  ? OrderStatus::New
                                  : OrderStatus::PartiallyFilled;

        result.incomingOrderToInsert = std::make_shared<Order>(std::move(incomingSell));
    }

    return result;
}

}  // namespace gridx::matching::matching