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
    return {};
}

std::vector<Trade> SameZoneMatcher::matchSell(
    Order& incomingSell,
    orderbook::ZoneOrderBook& zoneBook)
{
    return {};
}

} // namespace gridx::matching::matching