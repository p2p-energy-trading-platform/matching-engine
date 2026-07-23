#include "gridx/matching/expiry/ExpiryManager.hpp"

namespace gridx::matching::expiry {

using orderbook::BuyBook;
using orderbook::MarketBook;
using orderbook::SellBook;

void ExpiryManager::expireOrders(MarketBook& marketBook, Timestamp currentTime) const {
    if (!isMarketExpired(marketBook, currentTime)) {
        return;
    }

    for (auto& [gridZone, zoneOrderBook] : marketBook.zoneOrderBooks()) {
        expireBuyOrders(zoneOrderBook.buyBook());
        expireSellOrders(zoneOrderBook.sellBook());
    }
}

bool ExpiryManager::isMarketExpired(const MarketBook& marketBook, Timestamp currentTime) const {
    return currentTime >= marketBook.marketId().deliverySlotEnd();
}

void ExpiryManager::expireBuyOrders(BuyBook& buyBook) const {
    for (const auto& [price, orders] : buyBook.priceLevels()) {
        for (const auto& order : orders) {
            order->status = OrderStatus::Expired;
        }
    }

    buyBook.clear();
}

void ExpiryManager::expireSellOrders(SellBook& sellBook) const {
    for (const auto& [price, orders] : sellBook.priceLevels()) {
        for (const auto& order : orders) {
            order->status = OrderStatus::Expired;
        }
    }

    sellBook.clear();
}

}  // namespace gridx::matching::expiry