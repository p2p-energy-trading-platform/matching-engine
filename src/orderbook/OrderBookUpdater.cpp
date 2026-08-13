#include "gridx/matching/orderbook/OrderBookUpdater.hpp"

#include <stdexcept>

namespace gridx::matching::orderbook {

namespace {

void removeOrder(const OrderPtr& order, ZoneOrderBook& zoneOrderBook) {
    zoneOrderBook.removeFrontOrder(order->side, order->price);
}

void updateOrder(const OrderUpdate& update) {
    update.order->remainingQuantity = update.remainingQuantity;
    update.order->status = update.status;
}

}  // namespace

void OrderBookUpdater::apply(const MatchingResult& matchingResult,
                             ZoneOrderBook& zoneOrderBook) const {
    for (const auto& update : matchingResult.orderUpdates) {
        switch (update.action) {
            case OrderUpdateAction::Update:
                updateOrder(update);
                break;

            case OrderUpdateAction::Remove:
                removeOrder(update.order, zoneOrderBook);
                break;

            default:
                throw std::logic_error("Unknown order update action.");
        }
    }

    if (matchingResult.incomingOrderToInsert != nullptr) {
        zoneOrderBook.addOrder(matchingResult.incomingOrderToInsert);
    }
}

}  // namespace gridx::matching::orderbook