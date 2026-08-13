#include "gridx/matching/orderbook/ZoneOrderBook.hpp"

#include <mutex>

namespace gridx::matching::orderbook {

ZoneOrderBook::ZoneOrderBook(GridZoneId gridZone) : gridZone_(gridZone) {}
/**
 * Inserts an order into the appropriate book based on its side.
 */
void ZoneOrderBook::addOrder(const OrderPtr& order) {
    std::lock_guard lock(mutex_);

    if (order->side == Side::Buy) {
        buyBook_.addOrder(order);
    } else {
        sellBook_.addOrder(order);
    }
}

/**
 * Removes the oldest order from the requested side at the given price.
 */
void ZoneOrderBook::removeFrontOrder(Side side, Price price) {
    std::lock_guard lock(mutex_);

    if (side == Side::Buy) {
        buyBook_.removeFrontOrder(price);
    } else {
        sellBook_.removeFrontOrder(price);
    }
}

/**
 * Returns a snapshot of the resting orders on the requested side.
 */
std::vector<OrderPtr> ZoneOrderBook::snapshotOrders(Side side) const {
    std::lock_guard lock(mutex_);

    if (side == Side::Buy) {
        return buyBook_.snapshotOrders();
    }

    return sellBook_.snapshotOrders();
}

/**
 * Returns the number of resting orders on the requested side.
 */
std::size_t ZoneOrderBook::orderCount(Side side) const {
    std::lock_guard lock(mutex_);

    if (side == Side::Buy) {
        return buyBook_.orderCount();
    }

    return sellBook_.orderCount();
}

/**
 * Returns true when the requested side has no resting orders.
 */
bool ZoneOrderBook::empty(Side side) const {
    std::lock_guard lock(mutex_);

    if (side == Side::Buy) {
        return buyBook_.empty();
    }

    return sellBook_.empty();
}

/**
 * Removes all orders from both sides of the zone book.
 */
void ZoneOrderBook::clear() {
    std::lock_guard lock(mutex_);

    buyBook_.clear();
    sellBook_.clear();
}

/**
 * Returns the grid zone associated with this order book.
 */
GridZoneId ZoneOrderBook::gridZone() const noexcept {
    return gridZone_;
}

}  // namespace gridx::matching::orderbook