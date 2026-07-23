#pragma once

#include "gridx/matching/common/Types.hpp"
#include "gridx/matching/orderbook/MarketBook.hpp"

using namespace gridx::matching::orderbook;

namespace gridx::matching::expiry
{

class ExpiryManager
{
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
    bool isMarketExpired(
        const MarketBook& marketBook,
        Timestamp currentTime) const;
};

} // namespace gridx::matching::expiry