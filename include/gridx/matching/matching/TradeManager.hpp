#pragma once

#include "gridx/matching/domain/Order.hpp"
#include "gridx/matching/domain/Trade.hpp"

namespace gridx::matching
{

class TradeManager
{
public:
    TradeManager() = default;

    TradeManager(const TradeManager&) = delete;
    TradeManager& operator=(const TradeManager&) = delete;
    TradeManager(TradeManager&&) = delete;
    TradeManager& operator=(TradeManager&&) = delete;

    ~TradeManager() = default;

    [[nodiscard]]
    Trade createTrade(
        const Order& buyOrder,
        const Order& sellOrder,
        Quantity tradedQuantity,
        Price executionPrice,
        GridFee gridFee) const;
};

} // namespace gridx::matching