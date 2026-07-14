#pragma once

#include <vector>

#include "gridx/matching/domain/Order.hpp"
#include "gridx/matching/domain/Trade.hpp"

namespace gridx::matching
{

struct MatchingResult
{
    std::vector<Trade> trades;

    // Orders whose state changed during matching (e.g. remaining quantity or status).
    std::vector<Order> updatedOrders;

    // Remaining quantity of the incoming order after matching.
    Quantity remainingQuantity;
};

} // namespace gridx::matching