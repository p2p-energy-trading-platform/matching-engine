#pragma once

#include <chrono>
#include <cstdint>
#include <string>

namespace gridx::matching
{


// Identifier Types
using OrderId = std::uint64_t;
using TradeId = std::uint64_t;
using UserId = std::uint64_t;

// Market Types
using Price = double;
using Quantity = double;
using GridFee = double;

// Domain Types
using GridZoneId = std::string;

// Time Types
using Timestamp = std::chrono::system_clock::time_point;

// Enumerations

enum class Side
{
    Buy,
    Sell
};

enum class OrderType
{
    Limit
};

enum class OrderStatus
{
    New,
    PartiallyFilled,
    Filled,
    Cancelled,
    Expired
};

enum class ProductType
{
    Energy
};

} // namespace gridx::matching