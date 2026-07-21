#pragma once

#include <vector>

#include "gridx/matching/domain/Order.hpp"
#include "gridx/matching/domain/Trade.hpp"

namespace gridx::matching
{

class SameZoneMatcher;
class CrossZoneMatcher;
class TradeManager;
class MarketBook;

class MatchingEngine
{
public:
    MatchingEngine(
        SameZoneMatcher& sameZoneMatcher,
        TradeManager& tradeManager,
        MarketBook& marketBook);

    MatchingEngine(const MatchingEngine&) = delete;
    MatchingEngine& operator=(const MatchingEngine&) = delete;
    MatchingEngine(MatchingEngine&&) = delete;
    MatchingEngine& operator=(MatchingEngine&&) = delete;

    ~MatchingEngine() = default;

    [[nodiscard]]
    std::vector<Trade> process(Order incomingOrder);

private:
    SameZoneMatcher& m_sameZoneMatcher;
    TradeManager& m_tradeManager;
    MarketBook& m_marketBook;
};

} // namespace gridx::matching