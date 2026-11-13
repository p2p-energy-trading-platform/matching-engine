#pragma once

#include <string_view>

namespace gridx::matching::adapters::kafka::topics {

inline constexpr std::string_view kOrderAccepted{"order.accepted"};
inline constexpr std::string_view kTradeExecuted{"trade.executed"};
inline constexpr std::string_view kOrderState{"order.state"};

}  // namespace gridx::matching::adapters::kafka::topics