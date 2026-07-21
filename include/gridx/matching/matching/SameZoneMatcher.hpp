#pragma once

#include <vector>

#include "gridx/matching/domain/Order.hpp"
#include "gridx/matching/domain/Trade.hpp"

namespace gridx::matching::orderbook {

class MarketBook;
class ZoneOrderBook;
}  // namespace gridx::matching::orderbook

namespace gridx::matching::matching {

class TradeManager;

class SameZoneMatcher {
public:
    SameZoneMatcher(orderbook::MarketBook& marketBook, TradeManager& tradeManager);

    ~SameZoneMatcher() = default;

    SameZoneMatcher(const SameZoneMatcher&) = delete;
    SameZoneMatcher& operator=(const SameZoneMatcher&) = delete;

    SameZoneMatcher(SameZoneMatcher&&) noexcept = delete;
    SameZoneMatcher& operator=(SameZoneMatcher&&) noexcept = delete;

    /**
     * Matches an incoming order against opposite-side orders
     * within the same grid zone.
     */
    [[nodiscard]]
    std::vector<Trade> match(Order incomingOrder);

private:
    /**
     * Matches an incoming BUY order against the SELL book.
     */
    [[nodiscard]]
    std::vector<Trade> matchBuy(Order& incomingBuy, orderbook::ZoneOrderBook& zoneBook);

    /**
     * Matches an incoming SELL order against the BUY book.
     */
    [[nodiscard]]
    std::vector<Trade> matchSell(Order& incomingSell, orderbook::ZoneOrderBook& zoneBook);

private:
    /// Market order books used during matching.
    orderbook::MarketBook& m_marketBook;

    /// Creates Trade domain objects.
    TradeManager& m_tradeManager;
};

}  // namespace gridx::matching::matching