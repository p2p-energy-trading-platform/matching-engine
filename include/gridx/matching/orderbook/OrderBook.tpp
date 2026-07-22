#pragma once

namespace gridx::matching::orderbook
{

template <typename Comparator>
void OrderBook<Comparator>::addOrder(const OrderPtr& order)
{
    priceLevels_[order->price].push_back(order);
}

template <typename Comparator>
void OrderBook<Comparator>::removeFrontOrder(Price price)
{
    auto it = priceLevels_.find(price);

    if (it == priceLevels_.end())
    {
        return;
    }

    it->second.pop_front();

    if (it->second.empty())
    {
        priceLevels_.erase(it);
    }
}

template <typename Comparator>
const OrderQueue* OrderBook<Comparator>::bestPriceLevel() const
{
    if (priceLevels_.empty())
    {
        return nullptr;
    }

    return &priceLevels_.begin()->second;
}

template <typename Comparator>
const typename OrderBook<Comparator>::PriceLevels&
OrderBook<Comparator>::priceLevels() const noexcept
{
    return priceLevels_;
}

template <typename Comparator>
bool OrderBook<Comparator>::empty() const noexcept
{
    return priceLevels_.empty();
}

template <typename Comparator>
void OrderBook<Comparator>::clear() noexcept
{
    priceLevels_.clear();
}

} // namespace gridx::matching::orderbook