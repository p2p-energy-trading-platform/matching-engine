#pragma once

#include <deque>
#include <functional>
#include <map>
#include <memory>

#include "gridx/matching/domain/Order.hpp"

namespace gridx::matching::orderbook {

using OrderPtr = std::shared_ptr<Order>;
using OrderQueue = std::deque<OrderPtr>;

template <typename Comparator>
class OrderBook {
public:
    using PriceLevels = std::map<Price, OrderQueue, Comparator>;

    OrderBook() = default;
    ~OrderBook() = default;

    OrderBook(const OrderBook&) = delete;
    OrderBook& operator=(const OrderBook&) = delete;

    OrderBook(OrderBook&&) noexcept = default;
    OrderBook& operator=(OrderBook&&) noexcept = default;

    /**
     * Inserts an order into the appropriate price level.
     * Orders at the same price are maintained in FIFO order.
     */
    void addOrder(const OrderPtr& order);

    /**
     * Removes the oldest order from the specified price level.
     * If the price level becomes empty, it is removed.
     */
    void removeFrontOrder(Price price);

    /**
     * Returns the best price level.
     * Returns nullptr if the book is empty.
     */
    [[nodiscard]]
    const OrderQueue* bestPriceLevel() const;

    /**
     * Returns the oldest order at the best price level.
     * Returns nullptr if the order book is empty.
     */
    [[nodiscard]]
    OrderPtr bestOrder() const;

    /**
     * Returns the best available price.
     * Returns Price{} if the order book is empty.
     */
    [[nodiscard]]
    Price bestPrice() const;

    /**
     * Returns all price levels.
     */
    [[nodiscard]]
    const PriceLevels& priceLevels() const noexcept;

    /**
     * Returns true if the order book contains no orders.
     */
    [[nodiscard]]
    bool empty() const noexcept;

    /**
     * Removes all orders.
     */
    void clear() noexcept;

private:
    PriceLevels priceLevels_;
};

}  // namespace gridx::matching::orderbook

#include "gridx/matching/orderbook/OrderBook.tpp"