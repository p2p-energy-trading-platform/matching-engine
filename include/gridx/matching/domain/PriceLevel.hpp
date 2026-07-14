#pragma once

#include <deque>

#include "gridx/matching/domain/Order.hpp"

namespace gridx::matching
{

struct PriceLevel
{
    Price price;

    std::deque<Order> orders;
};

} // namespace gridx::matching