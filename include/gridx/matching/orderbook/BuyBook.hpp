#pragma once

#include <functional>

#include "gridx/matching/orderbook/OrderBook.hpp"

namespace gridx::matching::orderbook
{

class BuyBook : public OrderBook<std::greater<Price>>
{
public:
    BuyBook() = default;
    ~BuyBook() = default;

    BuyBook(const BuyBook&) = delete;
    BuyBook& operator=(const BuyBook&) = delete;

    BuyBook(BuyBook&&) noexcept = default;
    BuyBook& operator=(BuyBook&&) noexcept = default;
};

} // namespace gridx::matching::orderbook