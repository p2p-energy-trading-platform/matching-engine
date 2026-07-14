#include "gridx/matching/validation/OrderValidator.hpp"

#include <chrono>

namespace gridx::matching::validation {

ValidationResult OrderValidator::validate(const Order& order) const {
    if (order.orderId == 0) {
        return {false, ValidationError::InvalidOrderId};
    }

    if (order.userId == 0) {
        return {false, ValidationError::InvalidUserId};
    }

    if (order.orderType != OrderType::Limit) {
        return {false, ValidationError::InvalidOrderType};
    }
    

    if (order.price <= 0.0) {
        return {false, ValidationError::InvalidPrice};
    }

    if (order.quantity <= 0.0) {
        return {false, ValidationError::InvalidQuantity};
    }

    if (order.remainingQuantity < 0.0 || order.remainingQuantity > order.quantity) {
        return {false, ValidationError::InvalidRemainingQuantity};
    }

    if (order.expiresAt <= std::chrono::system_clock::now()) {
        return {false, ValidationError::OrderExpired};
    }

    return {true, ValidationError::None};
}

}  // namespace gridx::matching::validation