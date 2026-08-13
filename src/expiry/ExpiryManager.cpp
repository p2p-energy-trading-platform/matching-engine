#include "gridx/matching/expiry/ExpiryManager.hpp"

namespace gridx::matching::expiry {

using orderbook::MarketBook;

void ExpiryManager::expireOrders(MarketBook& marketBook, Timestamp currentTime) const {
    if (!isMarketExpired(marketBook, currentTime)) {
        return;
    }

    marketBook.withZoneOrderBooks([&](auto& zoneOrderBooks) {
        for (auto& [gridZone, zoneOrderBook] : zoneOrderBooks) {
            (void) gridZone;
            expireBuyOrders(zoneOrderBook, currentTime);
            expireSellOrders(zoneOrderBook, currentTime);
            zoneOrderBook.clear();
        }
    });
}

bool ExpiryManager::isMarketExpired(const MarketBook& marketBook, Timestamp currentTime) const {
    return currentTime >= marketBook.marketId().deliverySlotEnd();
}

void ExpiryManager::expireBuyOrders(orderbook::ZoneOrderBook& zoneOrderBook,
                                    Timestamp currentTime) const {
    (void) currentTime;

    for (const auto& order : zoneOrderBook.snapshotOrders(Side::Buy)) {
        order->status = OrderStatus::Expired;
    }
}

void ExpiryManager::expireSellOrders(orderbook::ZoneOrderBook& zoneOrderBook,
                                     Timestamp currentTime) const {
    (void) currentTime;

    for (const auto& order : zoneOrderBook.snapshotOrders(Side::Sell)) {
        order->status = OrderStatus::Expired;
    }
}

}  // namespace gridx::matching::expiry