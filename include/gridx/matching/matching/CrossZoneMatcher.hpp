#pragma once

#include "gridx/matching/domain/MatchingResult.hpp"
#include "gridx/matching/domain/Order.hpp"

namespace gridx::matching::orderbook {

class MarketBook;
class ZoneOrderBook;

}  // namespace gridx::matching::orderbook

namespace gridx::matching::config {
class GridTransferCache;
}  // namespace gridx::matching::config

namespace gridx::matching::matching {

class TradeManager;

class CrossZoneMatcher {
public:
    CrossZoneMatcher(orderbook::MarketBook& marketBook, TradeManager& tradeManager,
                     const config::GridTransferCache& gridTransferCache);

    ~CrossZoneMatcher() = default;

    CrossZoneMatcher(const CrossZoneMatcher&) = delete;
    CrossZoneMatcher& operator=(const CrossZoneMatcher&) = delete;

    CrossZoneMatcher(CrossZoneMatcher&&) noexcept = delete;
    CrossZoneMatcher& operator=(CrossZoneMatcher&&) noexcept = delete;

    /**
     * Matches an incoming order against opposite-side orders
     * in other grid zones when transfer rules permit.
     */
    [[nodiscard]]
    MatchingResult match(Order incomingOrder);

private:
    /**
     * Matches an incoming BUY order against SELL books
     * in all eligible grid zones.
     */
    [[nodiscard]]
    MatchingResult matchBuy(Order incomingBuy) const;

    /**
     * Matches an incoming SELL order against BUY books
     * in all eligible grid zones.
     */
    [[nodiscard]]
    MatchingResult matchSell(Order incomingSell) const;


    [[nodiscard]]
    static bool isBetterCandidate(const OrderPtr& candidateOrder, Price candidateEffectivePrice,
                                  const GridZoneId& candidateZone, const OrderPtr& currentBestOrder,
                                  Price currentBestEffectivePrice, const GridZoneId& incomingZone,  bool higherPriceWins);

private:
    /// Market order books used during matching.
    orderbook::MarketBook& m_marketBook;

    /// Creates Trade domain objects.
    TradeManager& m_tradeManager;

    /// In-memory grid transfer policy cache.
    const config::GridTransferCache& m_gridTransferCache;
};

}  // namespace gridx::matching::matching