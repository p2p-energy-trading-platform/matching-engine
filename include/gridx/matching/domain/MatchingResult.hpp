#pragma once

#include <memory>
#include <vector>

#include "gridx/matching/domain/Order.hpp"
#include "gridx/matching/domain/OrderUpdate.hpp"
#include "gridx/matching/domain/Trade.hpp"

namespace gridx::matching {

using OrderPtr = std::shared_ptr<Order>;

struct MatchingResult {
    // Trades generated during matching.
    std::vector<Trade> trades;

    // Changes that must be applied to the resting order book.
    std::vector<OrderUpdate> orderUpdates;

    // Remaining portion of the incoming order to insert into the book.
    // nullptr indicates the incoming order was completely filled.
    OrderPtr incomingOrderToInsert;
};

}  // namespace gridx::matching