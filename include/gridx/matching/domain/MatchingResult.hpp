#pragma once

#include <memory>
#include <vector>

#include "gridx/matching/domain/Order.hpp"
#include "gridx/matching/domain/Trade.hpp"

namespace gridx::matching {

using OrderPtr = std::shared_ptr<Order>;

struct MatchingResult {
    std::vector<Trade> trades;

    // Resting orders whose state changed during matching.
    std::vector<OrderPtr> updatedOrders;

    // Incoming order to insert into the order book if it was not fully matched.
    // nullptr indicates the incoming order was completely filled.
    std::shared_ptr<Order> incomingOrderToInsert;
};

}  // namespace gridx::matching