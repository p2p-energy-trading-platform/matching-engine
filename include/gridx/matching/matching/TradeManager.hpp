#pragma once

#include "gridx/matching/domain/Order.hpp"
#include "gridx/matching/domain/Trade.hpp"
#include "gridx/matching/domain/GridTransferRule.hpp"

namespace gridx::matching::matching {

class TradeManager {
public:
    TradeManager() = default;

    TradeManager(const TradeManager&) = delete;
    TradeManager& operator=(const TradeManager&) = delete;
    TradeManager(TradeManager&&) = delete;
    TradeManager& operator=(TradeManager&&) = delete;

    ~TradeManager() = default;

    [[nodiscard]]
    Trade createTrade(const Order& buyOrder, const Order& sellOrder, Quantity tradedQuantity,
                      Price executionPrice, const GridTransferRule& rule) const;
};

}  // namespace gridx::matching::matching