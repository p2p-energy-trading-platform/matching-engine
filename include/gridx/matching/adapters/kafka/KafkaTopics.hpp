#pragma once

#include <string_view>

namespace gridx::matching::adapters::kafka::topics {

inline constexpr std::string_view kOrderAccepted{"order.accepted"};
inline constexpr std::string_view kOrderExpired{"order.expired"};
inline constexpr std::string_view kOrderCancelled{"order.cancelled"};
inline constexpr std::string_view kTradeExecuted{"trade.executed"};

}  // namespace gridx::matching::adapters::kafka::topics