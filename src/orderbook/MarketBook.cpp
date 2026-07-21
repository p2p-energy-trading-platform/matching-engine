#include "gridx/matching/orderbook/MarketBook.hpp"

namespace gridx::matching::orderbook
{

MarketBook::MarketBook(const MarketId& marketId)
    : marketId_(marketId)
{
}

void MarketBook::addOrder(const OrderPtr& order)
{
    zoneOrderBook(order->gridZone).addOrder(order);
}

const MarketId& MarketBook::marketId() const noexcept
{
    return marketId_;
}

ZoneOrderBook& MarketBook::zoneOrderBook(GridZoneId gridZone)
{
    auto [it, inserted] =
        zoneOrderBooks_.try_emplace(gridZone, gridZone);

    return it->second;
}

const std::map<GridZoneId, ZoneOrderBook>&
MarketBook::zoneOrderBooks() const noexcept
{
    return zoneOrderBooks_;
}

} // namespace gridx::matching::orderbook