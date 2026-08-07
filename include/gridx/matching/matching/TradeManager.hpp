#pragma once

#include <memory>

#include "gridx/matching/domain/GridTransferRule.hpp"
#include "gridx/matching/domain/Order.hpp"
#include "gridx/matching/domain/Trade.hpp"
#include "gridx/matching/matching/TradeIdGenerator.hpp"

namespace gridx::matching::matching {

class TradeManager {
public:
    explicit TradeManager(std::unique_ptr<ITradeIdGenerator> tradeIdGenerator = nullptr);

    TradeManager(const TradeManager&) = delete;
    TradeManager& operator=(const TradeManager&) = delete;
    TradeManager(TradeManager&&) = delete;
    TradeManager& operator=(TradeManager&&) = delete;

    ~TradeManager() = default;

    [[nodiscard]]
    Trade createTrade(const Order& buyOrder, const Order& sellOrder, Quantity tradedQuantity,
                      Price executionPrice, const GridTransferRule& rule) const;

private:
    std::unique_ptr<ITradeIdGenerator> m_tradeIdGenerator;
};

}  // namespace gridx::matching::matching