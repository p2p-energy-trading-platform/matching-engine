#pragma once

#include "gridx/matching/orderbook/BuyBook.hpp"
#include "gridx/matching/orderbook/SellBook.hpp"

namespace gridx::matching::orderbook
{

class ZoneOrderBook
{
public:
    explicit ZoneOrderBook(GridZoneId gridZone);

    /**
     * Inserts an order into the appropriate book based on its side.
     */
    void addOrder(const OrderPtr& order);

    /**
     * Returns the grid zone associated with this order book.
     */
    GridZoneId gridZone() const noexcept;

    /**
     * Returns the buy order book.
     */
    BuyBook& buyBook() noexcept;
    const BuyBook& buyBook() const noexcept;

    /**
     * Returns the sell order book.
     */
    SellBook& sellBook() noexcept;
    const SellBook& sellBook() const noexcept;

private:
    GridZoneId gridZone_;

    BuyBook buyBook_;
    SellBook sellBook_;
};

} // namespace gridx::matching::orderbook