#pragma once

#include <shared_mutex>

namespace gridx::matching::orderbook {

template <typename Comparator>
typename OrderBook<Comparator>::PriceLevelSnapshot OrderBook<Comparator>::snapshotPriceLevels()
    const {
    std::shared_lock lock(mutex_);

    PriceLevelSnapshot snapshot;
    snapshot.reserve(priceLevels_.size());

    for (const auto& [price, orders] : priceLevels_) {
        snapshot.emplace_back(price, orders);
    }

    return snapshot;
}

template <typename Comparator>
std::vector<OrderPtr> OrderBook<Comparator>::snapshotOrders() const {
    std::shared_lock lock(mutex_);

    std::vector<OrderPtr> orders;

    for (const auto& [price, queue] : priceLevels_) {
        (void) price;
        orders.insert(orders.end(), queue.begin(), queue.end());
    }

    return orders;
}

template <typename Comparator>
void OrderBook<Comparator>::addOrder(const OrderPtr& order) {
    std::unique_lock lock(mutex_);

    priceLevels_[order->price].push_back(order);
}

template <typename Comparator>
void OrderBook<Comparator>::removeFrontOrder(Price price) {
    std::unique_lock lock(mutex_);

    auto it = priceLevels_.find(price);

    if (it == priceLevels_.end()) {
        return;
    }

    it->second.pop_front();

    if (it->second.empty()) {
        priceLevels_.erase(it);
    }
}

template <typename Comparator>
OrderPtr OrderBook<Comparator>::bestOrder() const {
    std::shared_lock lock(mutex_);

    if (priceLevels_.empty()) {
        return nullptr;
    }

    return priceLevels_.begin()->second.front();
}

template <typename Comparator>
Price OrderBook<Comparator>::bestPrice() const {
    std::shared_lock lock(mutex_);

    if (priceLevels_.empty()) {
        return Price{};
    }

    return priceLevels_.begin()->first;
}

template <typename Comparator>
std::size_t OrderBook<Comparator>::orderCount() const {
    std::shared_lock lock(mutex_);

    std::size_t count = 0;

    for (const auto& [price, queue] : priceLevels_) {
        (void) price;
        count += queue.size();
    }

    return count;
}

template <typename Comparator>
bool OrderBook<Comparator>::empty() const {
    std::shared_lock lock(mutex_);

    return priceLevels_.empty();
}

template <typename Comparator>
void OrderBook<Comparator>::clear() {
    std::unique_lock lock(mutex_);

    priceLevels_.clear();
}

}  // namespace gridx::matching::orderbook