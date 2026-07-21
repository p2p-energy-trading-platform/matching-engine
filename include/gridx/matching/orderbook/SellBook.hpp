#pragma once

#include <functional>

#include "gridx/matching/orderbook/OrderBook.hpp"

namespace gridx::matching::orderbook
{

class SellBook : public OrderBook<std::less<Price>>
{
public:
    SellBook() = default;
    ~SellBook() = default;

    SellBook(const SellBook&) = delete;
    SellBook& operator=(const SellBook&) = delete;

    SellBook(SellBook&&) noexcept = default;
    SellBook& operator=(SellBook&&) noexcept = default;
};

} // namespace gridx::matching::orderbook