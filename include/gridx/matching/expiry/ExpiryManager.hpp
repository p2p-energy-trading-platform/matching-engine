#pragma once

#include <vector>

#include "gridx/matching/common/Types.hpp"
#include "gridx/matching/domain/OrderUpdate.hpp"
#include "gridx/matching/orderbook/MarketBook.hpp"
#include "gridx/matching/orderbook/MarketBookManager.hpp"

namespace gridx::matching::expiry {

using orderbook::MarketBook;
using orderbook::MarketBookManager;
using orderbook::ZoneOrderBook;

class ExpiryManager {
public:
    ExpiryManager() = default;
    ~ExpiryManager() = default;

    ExpiryManager(const ExpiryManager&) = default;
    ExpiryManager& operator=(const ExpiryManager&) = default;
    ExpiryManager(ExpiryManager&&) noexcept = default;
    ExpiryManager& operator=(ExpiryManager&&) noexcept = default;

   
    std::vector<OrderUpdate> expireOrders(MarketBook& marketBook, Timestamp currentTime) const;

    std::vector<OrderUpdate> expireMarkets(MarketBookManager& marketBookManager,
                                           Timestamp currentTime) const;

private:
    [[nodiscard]]
    bool isMarketExpired(const MarketBook& marketBook, Timestamp currentTime) const;

    void expireSide(ZoneOrderBook& zoneOrderBook, Side side,
                    std::vector<OrderUpdate>& expiredUpdates) const;
};

}  // namespace gridx::matching::expiry
