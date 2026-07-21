#include <stdexcept>
#include <utility>

#include "gridx/matching/matching/SameZoneMatcher.hpp"
#include "gridx/matching/matching/TradeManager.hpp"
#include "gridx/matching/orderbook/MarketBook.hpp"
#include "gridx/matching/orderbook/ZoneOrderBook.hpp"

namespace gridx::matching::matching
{

SameZoneMatcher::SameZoneMatcher(
    orderbook::MarketBook& marketBook,
    TradeManager& tradeManager)
    : m_marketBook(marketBook),
      m_tradeManager(tradeManager)
{
}

std::vector<Trade> SameZoneMatcher::match(Order incomingOrder)
{
    auto& zoneBook =
        m_marketBook.zoneOrderBook(incomingOrder.gridZone);

    if (incomingOrder.side == Side::Buy)
    {
        return matchBuy(incomingOrder, zoneBook);
    }

    return matchSell(incomingOrder, zoneBook);
}

std::vector<Trade> SameZoneMatcher::matchBuy(
    Order& incomingBuy,
    orderbook::ZoneOrderBook& zoneBook)
{
    std::vector<Trade> trades;

    auto& sellBook = zoneBook.sellBook();

    while (!sellBook.empty() && incomingBuy.remainingQuantity > 0)
    {
        auto restingOrder = sellBook.bestOrder();

        // No resting order available.
        if (restingOrder == nullptr)
        {
            break;
        }

        // Stop if the best sell price is above the buyer's limit price.
        if (restingOrder->price > incomingBuy.price)
        {
            break;
        }

        const Quantity tradedQuantity =
            std::min(incomingBuy.remainingQuantity,
                     restingOrder->remainingQuantity);

        trades.push_back(
            m_tradeManager.createTrade(
                incomingBuy,
                *restingOrder,
                tradedQuantity,
                restingOrder->price,
                GridFee{0}));

        incomingBuy.remainingQuantity -= tradedQuantity;
        restingOrder->remainingQuantity -= tradedQuantity;

        // Remove fully matched resting order.
        if (restingOrder->remainingQuantity == 0)
        {
            sellBook.removeFrontOrder(restingOrder->price);
        }
    }

    // Store any remaining quantity back in the order book.
    if (incomingBuy.remainingQuantity > 0)
    {
        zoneBook.addOrder(std::make_shared<Order>(std::move(incomingBuy)));
    }

    return trades;
}

std::vector<Trade> SameZoneMatcher::matchSell(
    Order& incomingSell,
    orderbook::ZoneOrderBook& zoneBook)
{
    return {};
}

} // namespace gridx::matching::matching