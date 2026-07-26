#include <stdexcept>
#include <utility>

#include "gridx/matching/matching/SameZoneMatcher.hpp"
#include "gridx/matching/matching/TradeManager.hpp"
#include "gridx/matching/orderbook/MarketBook.hpp"
#include "gridx/matching/orderbook/ZoneOrderBook.hpp"

namespace gridx::matching::matching {

constexpr GridFee kSameZoneGridFee{0};

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
MatchingResult SameZoneMatcher::matchBuy(Order& incomingBuy,
                                         const orderbook::ZoneOrderBook& zoneBook) {
    MatchingResult result;

    auto& sellBook = zoneBook.sellBook();

    while (!sellBook.empty() && incomingBuy.remainingQuantity > 0) {
        auto restingOrder = sellBook.bestOrder();

        if (restingOrder == nullptr) {
            break;
        }

        if (restingOrder->price > incomingBuy.price) {
            break;
        }

        //
        const Quantity tradedQuantity =
            std::min(incomingBuy.remainingQuantity, restingOrder->remainingQuantity);

        // Create a Trade object for the matched orders.
        result.trades.push_back(m_tradeManager.createTrade(
            incomingBuy, *restingOrder, tradedQuantity, restingOrder->price, kSameZoneGridFee));

        incomingBuy.remainingQuantity -= tradedQuantity;
        // Update the resting order's remaining quantity and status.
        Order updatedOrder = *restingOrder;
        updatedOrder.remainingQuantity -= tradedQuantity;
        updatedOrder.status = updatedOrder.remainingQuantity == 0 ? OrderStatus::Filled
                                                                  : OrderStatus::PartiallyFilled;

        result.updatedOrders.push_back(std::move(updatedOrder));
    }

    // If the incoming order was not fully matched, it should be inserted into the order book.
    if (incomingBuy.remainingQuantity > 0) {
        incomingBuy.status = OrderStatus::PartiallyFilled;
        result.incomingOrderToInsert = std::make_shared<Order>(std::move(incomingBuy));
    }

    return result;
}

// Matches an incoming SELL order against the BUY book.
MatchingResult SameZoneMatcher::matchSell(Order& incomingSell,
                                          const orderbook::ZoneOrderBook& zoneBook) {
    MatchingResult result;

    auto& buyBook = zoneBook.buyBook();

    while (!buyBook.empty() && incomingSell.remainingQuantity > 0) {
        auto restingOrder = buyBook.bestOrder();

        if (restingOrder == nullptr) {
            break;
        }

        if (restingOrder->price < incomingSell.price) {
            break;
        }

        const Quantity tradedQuantity =
            std::min(incomingSell.remainingQuantity, restingOrder->remainingQuantity);
        // Create a Trade object for the matched orders.
        result.trades.push_back(m_tradeManager.createTrade(
            *restingOrder, incomingSell, tradedQuantity, restingOrder->price, kSameZoneGridFee));

        incomingSell.remainingQuantity -= tradedQuantity;
        // Update the resting order's remaining quantity and status.
        Order updatedOrder = *restingOrder;
        updatedOrder.remainingQuantity -= tradedQuantity;
        updatedOrder.status = updatedOrder.remainingQuantity == 0 ? OrderStatus::Filled
                                                                  : OrderStatus::PartiallyFilled;

        result.updatedOrders.push_back(std::move(updatedOrder));
    }
    // If the incoming order was not fully matched, it should be inserted into the order book.
    if (incomingSell.remainingQuantity > 0) {
        incomingSell.status = OrderStatus::PartiallyFilled;
        result.incomingOrderToInsert = std::make_shared<Order>(std::move(incomingSell));
    }

    return result;
}

}  // namespace gridx::matching::matching