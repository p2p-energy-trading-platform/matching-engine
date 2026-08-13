#pragma once

#include <map>
#include <shared_mutex>
#include <utility>

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
     * Finds an existing Zone Order Book.
     */
    [[nodiscard]]
    const ZoneOrderBook* findZoneOrderBook(GridZoneId gridZone) const;

    /**
     * Returns the number of active Zone Order Books.
     */
    [[nodiscard]]
    std::size_t zoneOrderBookCount() const;

    /**
     * Returns true if a Zone Order Book already exists for the given grid zone.
     */
    [[nodiscard]]
    bool hasZoneOrderBook(GridZoneId gridZone) const;

    /**
     * Runs a callback while holding a shared lock on the zone-book map.
     */
    template <typename Fn>
    decltype(auto) withZoneOrderBooks(Fn&& fn) const {
        std::shared_lock lock(mutex_);

        return std::forward<Fn>(fn)(zoneOrderBooks_);
    }

    /**
     * Runs a callback while holding an exclusive lock on the zone-book map.
     */
    template <typename Fn>
    decltype(auto) withZoneOrderBooks(Fn&& fn) {
        std::unique_lock lock(mutex_);

        return std::forward<Fn>(fn)(zoneOrderBooks_);
    }

private:
    ZoneOrderBook& zoneOrderBookUnlocked(GridZoneId gridZone);
    const ZoneOrderBook* findZoneOrderBookUnlocked(GridZoneId gridZone) const noexcept;

    MarketId marketId_;

    mutable std::shared_mutex mutex_;

    std::map<GridZoneId, ZoneOrderBook> zoneOrderBooks_;
};

}  // namespace gridx::matching::orderbook