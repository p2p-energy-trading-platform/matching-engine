#pragma once

#include <deque>
#include <map>
#include <memory>

#include "gridx/matching/domain/Order.hpp"

namespace gridx::matching::orderbook {

using OrderPtr = std::shared_ptr<Order>;
using OrderQueue = std::deque<OrderPtr>;

template <typename Comparator>
class RestingOrderIterator {
public:
    using PriceLevels = std::map<Price, OrderQueue, Comparator>;
    using PriceLevelIterator = typename PriceLevels::const_iterator;
    using QueueIterator = typename OrderQueue::const_iterator;

    RestingOrderIterator(PriceLevelIterator levelIt,
                         PriceLevelIterator levelEnd);

    ~RestingOrderIterator() = default;

    RestingOrderIterator(const RestingOrderIterator&) = default;
    RestingOrderIterator& operator=(const RestingOrderIterator&) = default;

    RestingOrderIterator(RestingOrderIterator&&) noexcept = default;
    RestingOrderIterator& operator=(RestingOrderIterator&&) noexcept = default;

    /**
     * Returns the current resting order.
     */
    [[nodiscard]]
    const OrderPtr& operator*() const noexcept;

    /**
     * Provides pointer-style access to the current resting order.
     */
    [[nodiscard]]
    const OrderPtr* operator->() const noexcept;

    /**
     * Advances to the next resting order in matching priority.
     */
    RestingOrderIterator& operator++();

    /**
     * Returns true if both iterators refer to the same position.
     */
    [[nodiscard]]
    bool operator==(const RestingOrderIterator& other) const noexcept;

    /**
     * Returns true if the iterators refer to different positions.
     */
    [[nodiscard]]
    bool operator!=(const RestingOrderIterator& other) const noexcept;

private:
    /**
     * Advances to the next non-empty price level.
     */
    void skipEmptyPriceLevels();

private:
    PriceLevelIterator m_levelIt;
    PriceLevelIterator m_levelEnd;
    QueueIterator m_queueIt;
};

}  // namespace gridx::matching::orderbook

#include "gridx/matching/orderbook/RestingOrderIterator.tpp"