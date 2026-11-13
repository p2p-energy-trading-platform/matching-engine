#include "gridx/matching/expiry/ExpiryManager.hpp"

namespace gridx::matching::expiry {

using orderbook::MarketBook;

std::vector<OrderUpdate> ExpiryManager::expireOrders(MarketBook& marketBook,
                                                     Timestamp currentTime) const {
    std::vector<OrderUpdate> expiredUpdates;

    if (!isMarketExpired(marketBook, currentTime)) {
        return expiredUpdates;
    }

    marketBook.withZoneOrderBooks([&](auto& zoneOrderBooks) {
        for (auto& [gridZone, zoneOrderBook] : zoneOrderBooks) {
            (void) gridZone;
            expireSide(zoneOrderBook, Side::Buy, expiredUpdates);
            expireSide(zoneOrderBook, Side::Sell, expiredUpdates);
            zoneOrderBook.clear();
        }
    });

    return expiredUpdates;
}

std::vector<OrderUpdate> ExpiryManager::expireMarkets(MarketBookManager& marketBookManager,
                                                      Timestamp currentTime) const {
    std::vector<OrderUpdate> expiredUpdates;
    std::vector<MarketId> expiredMarkets;

    marketBookManager.forEach([&](MarketBook& marketBook) {
        if (!isMarketExpired(marketBook, currentTime)) {
            return;
        }

        auto updates = expireOrders(marketBook, currentTime);
        expiredUpdates.insert(expiredUpdates.end(), std::make_move_iterator(updates.begin()),
                              std::make_move_iterator(updates.end()));

        expiredMarkets.push_back(marketBook.marketId());
    });

    // Erase after iterating so the map is not modified while it is being walked.
    for (const auto& marketId : expiredMarkets) {
        marketBookManager.erase(marketId);
    }

    return expiredUpdates;
}

bool ExpiryManager::isMarketExpired(const MarketBook& marketBook, Timestamp currentTime) const {
    return currentTime >= marketBook.marketId().deliverySlotEnd();
}

void ExpiryManager::expireSide(orderbook::ZoneOrderBook& zoneOrderBook, Side side,
                               std::vector<OrderUpdate>& expiredUpdates) const {
    for (const auto& order : zoneOrderBook.snapshotOrders(side)) {
        order->status = OrderStatus::Expired;

        expiredUpdates.push_back(OrderUpdate{.order = order,
                                             .remainingQuantity = order->remainingQuantity,
                                             .status = OrderStatus::Expired,
                                             .action = OrderUpdateAction::Remove});
    }
}

}  // namespace gridx::matching::expiry
