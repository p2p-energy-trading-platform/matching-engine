#include <gtest/gtest.h>

#include <memory>

#include "gridx/matching/orderbook/MarketBook.hpp"

using namespace gridx::matching;
using namespace gridx::matching::orderbook;

class MarketBookTest : public ::testing::Test
{
protected:
    MarketBook marketBook{MarketId{}};

    OrderPtr makeOrder(
        OrderId id,
        GridZoneId zone,
        Side side,
        Price price)
    {
        auto order = std::make_shared<Order>();

        order->orderId = id;
        order->userId = 1;

        order->marketId = {};
        order->gridZone = zone;

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