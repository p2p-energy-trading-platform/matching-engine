#include <gtest/gtest.h>

#include <memory>

#include "gridx/matching/orderbook/ZoneOrderBook.hpp"

using namespace gridx::matching;
using namespace gridx::matching::orderbook;

class ZoneOrderBookTest : public ::testing::Test {
protected:
    static constexpr GridZoneId ZoneId = 1;

    ZoneOrderBook zoneBook{ZoneId};

    OrderPtr makeOrder(OrderId id, Side side, Price price) {
        auto order = std::make_shared<Order>();

        order->orderId = id;
        order->userId = 1;

        order->marketId = {};
        order->gridZone = ZoneId;

        order->side = side;
        order->orderType = OrderType::Limit;
        order->status = OrderStatus::New;

        order->price = price;
        order->quantity = 100;
        order->remainingQuantity = 100;

        order->createdAt = Timestamp{};
        order->expiresAt = Timestamp{};

        return order;
    }
};

TEST_F(ZoneOrderBookTest, StoresGridZone) {
    EXPECT_EQ(zoneBook.gridZone(), ZoneId);
}

TEST_F(ZoneOrderBookTest, BuyOrderIsInsertedIntoBuyBook) {
    auto order = makeOrder(1, Side::Buy, 100);

    zoneBook.addOrder(order);

    EXPECT_FALSE(zoneBook.empty(Side::Buy));
    EXPECT_TRUE(zoneBook.empty(Side::Sell));

    ASSERT_EQ(zoneBook.orderCount(Side::Buy), 1);
}

TEST_F(ZoneOrderBookTest, SellOrderIsInsertedIntoSellBook) {
    auto order = makeOrder(1, Side::Sell, 100);

    zoneBook.addOrder(order);

    EXPECT_TRUE(zoneBook.empty(Side::Buy));
    EXPECT_FALSE(zoneBook.empty(Side::Sell));

    ASSERT_EQ(zoneBook.orderCount(Side::Sell), 1);
}

TEST_F(ZoneOrderBookTest, BuyAndSellOrdersAreStoredSeparately) {
    zoneBook.addOrder(makeOrder(1, Side::Buy, 100));
    zoneBook.addOrder(makeOrder(2, Side::Sell, 100));

    ASSERT_EQ(zoneBook.orderCount(Side::Buy), 1);
    ASSERT_EQ(zoneBook.orderCount(Side::Sell), 1);

    EXPECT_FALSE(zoneBook.empty(Side::Buy));
    EXPECT_FALSE(zoneBook.empty(Side::Sell));
}

TEST_F(ZoneOrderBookTest, MultipleBuyOrdersMaintainPriceOrdering) {
    zoneBook.addOrder(makeOrder(1, Side::Buy, 100));
    zoneBook.addOrder(makeOrder(2, Side::Buy, 110));
    zoneBook.addOrder(makeOrder(3, Side::Buy, 105));

    const auto orders = zoneBook.snapshotOrders(Side::Buy);
    auto it = orders.begin();

    ASSERT_NE(it, orders.end());

    EXPECT_EQ((*it)->price, 110);

    ++it;
    EXPECT_EQ((*it)->price, 105);

    ++it;
    EXPECT_EQ((*it)->price, 100);
}

TEST_F(ZoneOrderBookTest, MultipleSellOrdersMaintainPriceOrdering) {
    zoneBook.addOrder(makeOrder(1, Side::Sell, 100));
    zoneBook.addOrder(makeOrder(2, Side::Sell, 110));
    zoneBook.addOrder(makeOrder(3, Side::Sell, 95));

    const auto orders = zoneBook.snapshotOrders(Side::Sell);
    auto it = orders.begin();

    ASSERT_NE(it, orders.end());

    EXPECT_EQ((*it)->price, 95);

    ++it;
    EXPECT_EQ((*it)->price, 100);

    ++it;
    EXPECT_EQ((*it)->price, 110);
}