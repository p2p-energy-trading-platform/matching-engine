#include "gridx/matching/orderbook/MarketBook.hpp"

namespace gridx::matching::orderbook {

MarketBook::MarketBook(const MarketId& marketId) : marketId_(marketId) {}

/**
 * Inserts an order into the appropriate Zone Order Book.
 */
void MarketBook::addOrder(const OrderPtr& order) {
    zoneOrderBook(order->gridZone).addOrder(order);
}

/**
 * Returns the market identifier.
 */
const MarketId& MarketBook::marketId() const noexcept {
    return marketId_;
}

/**
 * Returns the Zone Order Book for the specified grid zone.
 * Creates one if it does not already exist.
 */
ZoneOrderBook& MarketBook::zoneOrderBook(GridZoneId gridZone) {
    auto [it, inserted] = zoneOrderBooks_.try_emplace(gridZone, gridZone);

    return it->second;
}

/**
 * Returns all Zone Order Books.
 */

 std::map<GridZoneId, ZoneOrderBook>& MarketBook::zoneOrderBooks() noexcept
{
    return zoneOrderBooks_;
}

const std::map<GridZoneId, ZoneOrderBook>& MarketBook::zoneOrderBooks() const noexcept {
    return zoneOrderBooks_;
}

}  // namespace gridx::matching::orderbook