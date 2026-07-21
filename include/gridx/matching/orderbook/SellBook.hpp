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

};

} // namespace gridx::matching::orderbook