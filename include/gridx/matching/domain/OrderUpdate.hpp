#pragma once

#include <memory>

#include "gridx/matching/domain/Order.hpp"

namespace gridx::matching {

using OrderPtr = std::shared_ptr<Order>;

enum class OrderUpdateAction { Modify, Remove };

struct OrderUpdate {
    // Order currently stored in the order book.
    OrderPtr originalOrder;

    // Updated state after matching.
    Order updatedOrder;

    // Action to apply to the order book.
    OrderUpdateAction action;
};

}  // namespace gridx::matching