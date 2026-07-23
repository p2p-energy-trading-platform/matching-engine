#pragma once

#include <map>

#include "gridx/matching/domain/MarketId.hpp"
#include "gridx/matching/orderbook/ZoneOrderBook.hpp"

namespace gridx::matching::orderbook {

class MarketBook {
public:
    explicit MarketBook(const MarketId& marketId);

    /**
     * Inserts an order into the appropriate Zone Order Book.
     */
    void addOrder(const OrderPtr& order);

    /**
     * Returns the market identifier.
     */
    const MarketId& marketId() const noexcept;

    /**
     * Returns the Zone Order Book for the specified grid zone.
     * Creates one if it does not already exist.
     */
    ZoneOrderBook& zoneOrderBook(GridZoneId gridZone);

    /**
     * Returns all Zone Order Books.
     */
    std::map<GridZoneId, ZoneOrderBook>& zoneOrderBooks() noexcept;
    
    const std::map<GridZoneId, ZoneOrderBook>& zoneOrderBooks() const noexcept;


private:
    MarketId marketId_;

    std::map<GridZoneId, ZoneOrderBook> zoneOrderBooks_;
};

}  // namespace gridx::matching::orderbook