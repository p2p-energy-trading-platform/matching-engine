#pragma once

#include <functional>

#include "gridx/matching/orderbook/OrderBook.hpp"

namespace gridx::matching::orderbook {

class BuyBook : public OrderBook<std::greater<Price>> {
public:
    BuyBook() = default;
    ~BuyBook() = default;
};

}  // namespace gridx::matching::orderbook