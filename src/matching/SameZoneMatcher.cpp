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
std::vector<Trade> SameZoneMatcher::match(Order incomingOrder) {
    auto& zoneBook = m_marketBook.zoneOrderBook(incomingOrder.gridZone);

    if (incomingOrder.side == Side::Buy) {
        return matchBuy(incomingOrder, zoneBook);
    }

    return matchSell(incomingOrder, zoneBook);
}

// Matches an incoming BUY order against the SELL book.
std::vector<Trade> SameZoneMatcher::matchBuy(Order& incomingBuy,
                                             orderbook::ZoneOrderBook& zoneBook) {
    std::vector<Trade> trades;

    auto& sellBook = zoneBook.sellBook();

    while (!sellBook.empty() && incomingBuy.remainingQuantity > 0) {
        auto restingOrder = sellBook.bestOrder();

        // No resting order available.
        if (restingOrder == nullptr) {
            break;
        }

        // Stop if the best sell price is above the buyer's limit price.
        if (restingOrder->price > incomingBuy.price) {
            break;
        }

        const Quantity tradedQuantity =
            std::min(incomingBuy.remainingQuantity, restingOrder->remainingQuantity);

        trades.push_back(m_tradeManager.createTrade(incomingBuy, *restingOrder, tradedQuantity,
                                                    restingOrder->price, kSameZoneGridFee));

        incomingBuy.remainingQuantity -= tradedQuantity;
        restingOrder->remainingQuantity -= tradedQuantity;

        // Remove fully matched resting order.
        if (restingOrder->remainingQuantity == 0) {
            sellBook.removeFrontOrder(restingOrder->price);
        }
    }

    // Store any remaining quantity back in the order book.
    if (incomingBuy.remainingQuantity > 0) {
        zoneBook.addOrder(std::make_shared<Order>(std::move(incomingBuy)));
    }

    return trades;
}

// Matches an incoming SELL order against the BUY book.
std::vector<Trade> SameZoneMatcher::matchSell(Order& incomingSell,
                                              orderbook::ZoneOrderBook& zoneBook) {
    std::vector<Trade> trades;

    auto& buyBook = zoneBook.buyBook();

    while (!buyBook.empty() && incomingSell.remainingQuantity > 0) {
        auto restingOrder = buyBook.bestOrder();

        if (restingOrder == nullptr) {
            break;
        }

        // Stop if the best buy price is below the seller's limit price.
        if (restingOrder->price < incomingSell.price) {
            break;
        }

        const Quantity tradedQuantity =
            std::min(incomingSell.remainingQuantity, restingOrder->remainingQuantity);

        trades.push_back(m_tradeManager.createTrade(*restingOrder, incomingSell, tradedQuantity,
                                                    restingOrder->price, kSameZoneGridFee));

        incomingSell.remainingQuantity -= tradedQuantity;
        restingOrder->remainingQuantity -= tradedQuantity;

        if (restingOrder->remainingQuantity == 0) {
            buyBook.removeFrontOrder(restingOrder->price);
        }
    }

    // Store any remaining quantity back in the order book.
    if (incomingSell.remainingQuantity > 0) {
        zoneBook.addOrder(std::make_shared<Order>(std::move(incomingSell)));
    }

    return trades;
}

}  // namespace gridx::matching::matching