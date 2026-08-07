#pragma once

#include "gridx/matching/domain/Trade.hpp"

#include "gridx/trade/v1/trade_events.pb.h"

namespace gridx::matching::adapters::kafka {

class TradeEventMapper {
public:
    [[nodiscard]]
    gridx::trade::v1::TradeExecuted toProtobuf(const Trade& trade) const;
};

}  // namespace gridx::matching::adapters::kafka