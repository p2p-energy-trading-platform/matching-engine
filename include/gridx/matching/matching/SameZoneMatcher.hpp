#pragma once

#include <vector>

#include "gridx/matching/domain/Order.hpp"
#include "gridx/matching/domain/Trade.hpp"

namespace gridx::matching
{

class MarketBook;
class TradeManager;
class ZoneOrderBook;

class SameZoneMatcher
{
public:
    SameZoneMatcher(
        MarketBook& marketBook,
        TradeManager& tradeManager);

    SameZoneMatcher(const SameZoneMatcher&) = delete;
    SameZoneMatcher& operator=(const SameZoneMatcher&) = delete;
    SameZoneMatcher(SameZoneMatcher&&) = delete;
    SameZoneMatcher& operator=(SameZoneMatcher&&) = delete;

    ~SameZoneMatcher() = default;

    [[nodiscard]]
    std::vector<Trade> match(Order incomingOrder);

private:
    [[nodiscard]]
    std::vector<Trade> matchBuy(
        Order& incomingBuy,
        ZoneOrderBook& zoneBook);

    [[nodiscard]]
    std::vector<Trade> matchSell(
        Order& incomingSell,
        ZoneOrderBook& zoneBook);

private:
    MarketBook& m_marketBook;
    TradeManager& m_tradeManager;
};

} // namespace gridx::matching