#include "gridx/matching/expiry/ExpiryManager.hpp"


namespace gridx::matching::expiry
{

void ExpiryManager::expireOrders(
    orderbook::MarketBook& marketBook,
    Timestamp currentTime) const
{
    if (!isMarketExpired(marketBook, currentTime))
    {
        return;
    }

    for (auto& [_, zoneOrderBook] : marketBook.zoneOrderBooks())
    {
        expireBuyOrders(zoneOrderBook.buyBook());
        expireSellOrders(zoneOrderBook.sellBook());
    }
}

bool ExpiryManager::isMarketExpired(
    const orderbook::MarketBook& marketBook,
    Timestamp currentTime) const
{
    return currentTime >= marketBook.marketId().deliverySlotEnd();
}

void ExpiryManager::expireBuyOrders(BuyBook& buyBook) const
{
    for (const auto& [_, orders] : buyBook.priceLevels())
    {
        for (const auto& order : orders)
        {
            order->status = OrderStatus::Expired;
        }
    }

    buyBook.clear();
}

void ExpiryManager::expireSellOrders(orderbook::SellBook& sellBook) const
{
    for (const auto& [_, orders] : sellBook.priceLevels())
    {
        for (const auto& order : orders)
        {
            order->status = OrderStatus::Expired;
        }
    }

    sellBook.clear();
}

} // namespace gridx::matching::expiry