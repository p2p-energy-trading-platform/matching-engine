#pragma once

#include <deque>

#include "gridx/matching/domain/Order.hpp"

namespace gridx::matching
{

struct PriceLevel
{
    Price price;

    // std::deque<Order> orders;

    // NOTE:
    // Order storage is intentionally omitted from the domain model.
    // The OrderBook implementation will determine the appropriate
    // container (e.g. std::deque<Order> or std::deque<std::shared_ptr<Order>>)
    // based on performance and ownership requirements.
};

} // namespace gridx::matching