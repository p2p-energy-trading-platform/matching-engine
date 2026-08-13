#pragma once

#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <vector>

#include "gridx/matching/domain/Order.hpp"

namespace gridx::matching::orderbook {

using OrderPtr = std::shared_ptr<Order>;
using OrderQueue = std::deque<OrderPtr>;

template <typename Comparator>
class OrderBook {
public:
    using PriceLevels = std::map<Price, OrderQueue, Comparator>;
    using PriceLevelSnapshot = std::vector<std::pair<Price, OrderQueue>>;

    OrderBook() = default;
    ~OrderBook() = default;

    OrderBook(const OrderBook&) = delete;
    OrderBook& operator=(const OrderBook&) = delete;

    OrderBook(OrderBook&&) noexcept = default;
    OrderBook& operator=(OrderBook&&) noexcept = default;

    /**
     * Returns all price levels in price-time priority.
     */
    [[nodiscard]]
    PriceLevelSnapshot snapshotPriceLevels() const;

    /**
     * Returns all resting orders in price-time priority.
     */
    [[nodiscard]]
    std::vector<OrderPtr> snapshotOrders() const;

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
     * Returns the best available price.
     * Returns nullptr if the book is empty.
     */
    [[nodiscard]]
    OrderPtr bestOrder() const;

    /**
     * Returns the best available price.
     * Returns Price{} if the book is empty.
     */
    [[nodiscard]]
    Price bestPrice() const;

    /**
     * Returns the number of resting orders in the book.
     */
    [[nodiscard]]
    std::size_t orderCount() const;

    /**
     * Returns true if the order book contains no orders.
     */
    [[nodiscard]]
    bool empty() const;

    /**
     * Removes all orders.
     */
    void clear();

private:
    mutable std::shared_mutex mutex_;
    PriceLevels priceLevels_;
};

}  // namespace gridx::matching::orderbook

#include "gridx/matching/orderbook/OrderBook.tpp"