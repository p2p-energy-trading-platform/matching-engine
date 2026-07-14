#pragma once

namespace gridx::matching::validation
{

enum class ValidationError
{
    None,

    // Required fields
    InvalidOrderId,
    InvalidUserId,
    InvalidMarketId,
    InvalidGridZone,

    // Field validation
    InvalidOrderType,
    InvalidPrice,
    InvalidQuantity,
    InvalidRemainingQuantity,

    // Dyanamic Time-sensitive validation
    OrderExpired,
    MarketClosed
};

struct ValidationResult
{
    bool valid{true};
    ValidationError error{ValidationError::None};
};

} // namespace gridx::matching::validation