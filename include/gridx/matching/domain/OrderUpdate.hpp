#pragma once

#include <memory>

#include "gridx/matching/domain/Order.hpp"

namespace gridx::matching {

using OrderPtr = std::shared_ptr<Order>;

enum class OrderUpdateAction { Update, Remove };

struct OrderUpdate {
    // Order currently stored in the order book.
    OrderPtr order;

    // New state after matching.
    Quantity remainingQuantity;
    OrderStatus status;

    // Action to apply to the order book.
    OrderUpdateAction action;
};

}  // namespace gridx::matching