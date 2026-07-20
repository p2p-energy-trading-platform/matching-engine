#pragma once

#include <chrono>
#include <cstdint>
#include <string>

namespace gridx::matching {

// Identifier Types
using OrderId = std::uint64_t;
using TradeId = std::uint64_t;
using UserId = std::uint64_t;

// Market Types
using Price = std::int64_t;     // 1 AED = 1,000,000 micro-AED
using Quantity = std::int64_t;  // 1 kWh = 1000 Wh
using GridFee = std::int64_t;   // 1 AED = 1,000,000 micro-AED

// Domain Types
using GridZoneId = std::uint32_t;

using GridRuleVersion = std::uint32_t;

// Time Types
using Timestamp = std::chrono::system_clock::time_point;

// Enumerations

enum class Side { Buy, Sell };

/* Note: OrderType is currently limited to Limit orders,
but can be extended  to support other order types such as Market.
*/
enum class OrderType { Limit };

enum class OrderStatus { New, PartiallyFilled, Filled, Cancelled, Expired };

}  // namespace gridx::matching