#include <gtest/gtest.h>

#include <memory>

#include "gridx/matching/orderbook/ZoneOrderBook.hpp"

using namespace gridx::matching;
using namespace gridx::matching::orderbook;

class ZoneOrderBookTest : public ::testing::Test
{
protected:
    static constexpr GridZoneId ZoneId = 1;

    ZoneOrderBook zoneBook{ZoneId};

    OrderPtr makeOrder(
        OrderId id,
        Side side,
        Price price)
    {
        auto order = std::make_shared<Order>();

        order->orderId = id;
        order->userId = 1;

        order->marketId = {};
        order->gridZone = ZoneId;

        order->side = side;
        order->orderType = OrderType::Limit;
        order->status = OrderStatus::Pending;

        order->price = price;
        order->quantity = 100;
        order->remainingQuantity = 100;

        order->createdAt = Timestamp{};
        order->expiresAt = Timestamp{};

        return order;
    }
};