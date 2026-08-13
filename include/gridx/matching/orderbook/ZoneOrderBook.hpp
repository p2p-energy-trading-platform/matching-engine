#pragma once

#include <mutex>
#include <vector>

#include "gridx/matching/orderbook/BuyBook.hpp"
#include "gridx/matching/orderbook/SellBook.hpp"

namespace gridx::matching::orderbook {

class ZoneOrderBook {
public:
    explicit ZoneOrderBook(GridZoneId gridZone);

    /**
     * Inserts an order into the appropriate book based on its side.
     */
    void addOrder(const OrderPtr& order);

    /**
     * Removes the oldest order from the requested side at the given price.
     */
    void removeFrontOrder(Side side, Price price);

    /**
     * Returns a snapshot of the resting orders on the requested side.
     */
    [[nodiscard]]
    std::vector<OrderPtr> snapshotOrders(Side side) const;

    /**
     * Returns the number of resting orders on the requested side.
     */
    [[nodiscard]]
    std::size_t orderCount(Side side) const;

    /**
     * Returns true when the requested side has no resting orders.
     */
    [[nodiscard]]
    bool empty(Side side) const;

    /**
     * Removes all orders from both sides of the zone book.
     */
    void clear();

    /**
     * Returns the grid zone associated with this order book.
     */
    GridZoneId gridZone() const noexcept;

private:
    GridZoneId gridZone_;

    mutable std::mutex mutex_;

    BuyBook buyBook_;
    SellBook sellBook_;
};

}  // namespace gridx::matching::orderbook