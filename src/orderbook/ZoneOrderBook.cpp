#include "gridx/matching/orderbook/ZoneOrderBook.hpp"

namespace gridx::matching::orderbook {

ZoneOrderBook::ZoneOrderBook(GridZoneId gridZone) : gridZone_(gridZone) {}
/**
 * Inserts an order into the appropriate book based on its side.
 */
void ZoneOrderBook::addOrder(const OrderPtr& order) {
    if (order->side == Side::Buy) {
        buyBook_.addOrder(order);
    } else {
        sellBook_.addOrder(order);
    }
}

/**
 * Returns the grid zone associated with this order book.
 */
GridZoneId ZoneOrderBook::gridZone() const noexcept {
    return gridZone_;
}

/**
 * Returns the buy order book.
 */
BuyBook& ZoneOrderBook::buyBook() noexcept {
    return buyBook_;
}

/**
 * Returns the buy order book (const version).
 */
const BuyBook& ZoneOrderBook::buyBook() const noexcept {
    return buyBook_;
}

/**
 * Returns the sell order book.
 */
SellBook& ZoneOrderBook::sellBook() noexcept {
    return sellBook_;
}

/**
 * Returns the sell order book (const version).
 */
const SellBook& ZoneOrderBook::sellBook() const noexcept {
    return sellBook_;
}

}  // namespace gridx::matching::orderbook