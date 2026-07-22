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

    EXPECT_FALSE(zoneBook.buyBook().empty());
    EXPECT_TRUE(zoneBook.sellBook().empty());

    ASSERT_EQ(zoneBook.buyBook().priceLevels().size(), 1);
}

TEST_F(ZoneOrderBookTest, SellOrderIsInsertedIntoSellBook) {
    auto order = makeOrder(1, Side::Sell, 100);

    zoneBook.addOrder(order);

    EXPECT_TRUE(zoneBook.buyBook().empty());
    EXPECT_FALSE(zoneBook.sellBook().empty());

    ASSERT_EQ(zoneBook.sellBook().priceLevels().size(), 1);
}

TEST_F(ZoneOrderBookTest, BuyAndSellOrdersAreStoredSeparately) {
    zoneBook.addOrder(makeOrder(1, Side::Buy, 100));
    zoneBook.addOrder(makeOrder(2, Side::Sell, 100));

    ASSERT_EQ(zoneBook.buyBook().priceLevels().size(), 1);
    ASSERT_EQ(zoneBook.sellBook().priceLevels().size(), 1);

    EXPECT_FALSE(zoneBook.buyBook().empty());
    EXPECT_FALSE(zoneBook.sellBook().empty());
}

TEST_F(ZoneOrderBookTest, MultipleBuyOrdersMaintainPriceOrdering) {
    zoneBook.addOrder(makeOrder(1, Side::Buy, 100));
    zoneBook.addOrder(makeOrder(2, Side::Buy, 110));
    zoneBook.addOrder(makeOrder(3, Side::Buy, 105));

    auto it = zoneBook.buyBook().priceLevels().begin();

    ASSERT_NE(it, zoneBook.buyBook().priceLevels().end());

    EXPECT_EQ(it->first, 110);

    ++it;
    EXPECT_EQ(it->first, 105);

    ++it;
    EXPECT_EQ(it->first, 100);
}

TEST_F(ZoneOrderBookTest, MultipleSellOrdersMaintainPriceOrdering) {
    zoneBook.addOrder(makeOrder(1, Side::Sell, 100));
    zoneBook.addOrder(makeOrder(2, Side::Sell, 110));
    zoneBook.addOrder(makeOrder(3, Side::Sell, 95));

    auto it = zoneBook.sellBook().priceLevels().begin();

    ASSERT_NE(it, zoneBook.sellBook().priceLevels().end());

    EXPECT_EQ(it->first, 95);

    ++it;
    EXPECT_EQ(it->first, 100);

    ++it;
    EXPECT_EQ(it->first, 110);
}