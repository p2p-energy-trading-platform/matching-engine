#include <gtest/gtest.h>

#include "gridx/matching/orderbook/OrderBookUpdater.hpp"
#include "gridx/matching/orderbook/ZoneOrderBook.hpp"

using namespace gridx::matching;
using namespace gridx::matching::orderbook;

class OrderBookUpdaterTest : public ::testing::Test {
protected:
    OrderBookUpdater updater;
    ZoneOrderBook zoneOrderBook;

    static OrderPtr createBuyOrder(
        OrderId id,
        Price price,
        Quantity quantity,
        Quantity remaining)
    {
        auto order = std::make_shared<Order>();

        order->orderId = id;
        order->side = Side::Buy;
        order->price = price;
        order->quantity = quantity;
        order->remainingQuantity = remaining;
        order->status = OrderStatus::New;

        return order;
    }

    static OrderPtr createSellOrder(
        OrderId id,
        Price price,
        Quantity quantity,
        Quantity remaining)
    {
        auto order = std::make_shared<Order>();

        order->orderId = id;
        order->side = Side::Sell;
        order->price = price;
        order->quantity = quantity;
        order->remainingQuantity = remaining;
        order->status = OrderStatus::New;

        return order;
    }
};

// Verify that a fully matched order is removed from the order book.
TEST_F(OrderBookUpdaterTest, UpdatesPartiallyMatchedOrder)
{
    auto order = createBuyOrder(1, 100, 10, 10);

    zoneOrderBook.addOrder(order);

    MatchingResult result;

    result.orderUpdates.push_back(OrderUpdate{
        .order = order,
        .remainingQuantity = 4,
        .status = OrderStatus::PartiallyFilled,
        .action = OrderUpdateAction::Update
    });

    updater.apply(result, zoneOrderBook);

    EXPECT_EQ(order->remainingQuantity, 4);
    EXPECT_EQ(order->status, OrderStatus::PartiallyFilled);
}

// Verify that a fully matched BUY order is removed from the order book.
TEST_F(OrderBookUpdaterTest, RemovesFilledBuyOrder)
{
    auto order = createBuyOrder(1, 100, 10, 10);

    zoneOrderBook.addOrder(order);

    MatchingResult result;

    result.orderUpdates.push_back(OrderUpdate{
        .order = order,
        .remainingQuantity = 0,
        .status = OrderStatus::Filled,
        .action = OrderUpdateAction::Remove
    });

    updater.apply(result, zoneOrderBook);

    EXPECT_TRUE(zoneOrderBook.buyBook().empty());
}
// Verify that a fully matched order is removed from the order book.
TEST_F(OrderBookUpdaterTest, RemovesFilledSellOrder)
{
    auto order = createSellOrder(1, 100, 10, 10);

    zoneOrderBook.addOrder(order);

    MatchingResult result;

    result.orderUpdates.push_back(OrderUpdate{
        .order = order,
        .remainingQuantity = 0,
        .status = OrderStatus::Filled,
        .action = OrderUpdateAction::Remove
    });

    updater.apply(result, zoneOrderBook);

    EXPECT_TRUE(zoneOrderBook.sellBook().empty());
}

// Verify that a remaining incoming order is added to the order book.
TEST_F(OrderBookUpdaterTest, InsertsRemainingIncomingOrder)
{
    MatchingResult result;

    result.incomingOrderToInsert =
        createBuyOrder(2, 110, 10, 5);

    updater.apply(result, zoneOrderBook);

    EXPECT_FALSE(zoneOrderBook.buyBook().empty());
}

// Verify that an existing order is updated and a new incoming order is inserted.
TEST_F(OrderBookUpdaterTest, UpdatesExistingOrderAndInsertsIncomingOrder)
{
    auto existing = createSellOrder(1, 100, 10, 10);

    zoneOrderBook.addOrder(existing);

    MatchingResult result;

    result.orderUpdates.push_back(OrderUpdate{
        .order = existing,
        .remainingQuantity = 3,
        .status = OrderStatus::PartiallyFilled,
        .action = OrderUpdateAction::Update
    });

    result.incomingOrderToInsert =
        createBuyOrder(2, 100, 7, 7);

    updater.apply(result, zoneOrderBook);

    EXPECT_EQ(existing->remainingQuantity, 3);
    EXPECT_EQ(existing->status, OrderStatus::PartiallyFilled);

    EXPECT_FALSE(zoneOrderBook.buyBook().empty());
}

// Verify that an empty MatchingResult does not cause any errors.
TEST_F(OrderBookUpdaterTest, DoesNothingWhenMatchingResultIsEmpty)
{
    MatchingResult result;

    EXPECT_NO_THROW(
        updater.apply(result, zoneOrderBook));
}
