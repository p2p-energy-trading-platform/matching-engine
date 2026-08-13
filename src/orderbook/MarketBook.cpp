#include "gridx/matching/orderbook/MarketBook.hpp"

#include <mutex>
#include <shared_mutex>

namespace gridx::matching::orderbook {

MarketBook::MarketBook(const MarketId& marketId) : marketId_(marketId) {}

/**
 * Inserts an order into the appropriate Zone Order Book.
 */
void MarketBook::addOrder(const OrderPtr& order) {
    std::unique_lock lock(mutex_);

    zoneOrderBookUnlocked(order->gridZone).addOrder(order);
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
    std::unique_lock lock(mutex_);

    return zoneOrderBookUnlocked(gridZone);
}

const ZoneOrderBook* MarketBook::findZoneOrderBook(GridZoneId gridZone) const {
    std::shared_lock lock(mutex_);

    return findZoneOrderBookUnlocked(gridZone);
}

std::size_t MarketBook::zoneOrderBookCount() const {
    std::shared_lock lock(mutex_);

    return zoneOrderBooks_.size();
}

bool MarketBook::hasZoneOrderBook(GridZoneId gridZone) const {
    std::shared_lock lock(mutex_);

    return zoneOrderBooks_.find(gridZone) != zoneOrderBooks_.end();
}

ZoneOrderBook& MarketBook::zoneOrderBookUnlocked(GridZoneId gridZone) {
    auto [it, inserted] = zoneOrderBooks_.try_emplace(gridZone, gridZone);

    return it->second;
}

const ZoneOrderBook* MarketBook::findZoneOrderBookUnlocked(GridZoneId gridZone) const noexcept {
    const auto iterator = zoneOrderBooks_.find(gridZone);

    if (iterator == zoneOrderBooks_.end()) {
        return nullptr;
    }

    return &iterator->second;
}

}  // namespace gridx::matching::orderbook