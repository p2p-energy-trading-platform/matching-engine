#pragma once

#include "gridx/matching/common/Types.hpp"
#include "gridx/matching/orderbook/MarketBook.hpp"

namespace gridx::matching::expiry {

using orderbook::BuyBook;
using orderbook::MarketBook;
using orderbook::SellBook;

class ExpiryManager {
public:
    ExpiryManager() = default;
    ~ExpiryManager() = default;

    ExpiryManager(const ExpiryManager&) = default;
    ExpiryManager& operator=(const ExpiryManager&) = default;
    ExpiryManager(ExpiryManager&&) noexcept = default;
    ExpiryManager& operator=(ExpiryManager&&) noexcept = default;

    void expireOrders(MarketBook& marketBook, Timestamp currentTime) const;

private:
    [[nodiscard]]
    bool isMarketExpired(const MarketBook& marketBook, Timestamp currentTime) const;

    void expireBuyOrders(BuyBook& buyBook) const;

    void expireSellOrders(SellBook& sellBook) const;
};

}  // namespace gridx::matching::expiry