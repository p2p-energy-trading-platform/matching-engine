#include "gridx/matching/orderbook/ZoneOrderBook.hpp"

namespace gridx::matching::orderbook
{

ZoneOrderBook::ZoneOrderBook(GridZoneId gridZone)
    : gridZone_(gridZone)
{
}

void ZoneOrderBook::addOrder(const OrderPtr& order)
{
    if (order->side == Side::Buy)
    {
        buyBook_.addOrder(order);
    }
    else
    {
        sellBook_.addOrder(order);
    }
}

GridZoneId ZoneOrderBook::gridZone() const noexcept
{
    return gridZone_;
}

BuyBook& ZoneOrderBook::buyBook() noexcept
{
    return buyBook_;
}

const BuyBook& ZoneOrderBook::buyBook() const noexcept
{
    return buyBook_;
}

SellBook& ZoneOrderBook::sellBook() noexcept
{
    return sellBook_;
}

const SellBook& ZoneOrderBook::sellBook() const noexcept
{
    return sellBook_;
}

} // namespace gridx::matching::orderbook