#include <gtest/gtest.h>

#include <memory>

#include "gridx/matching/orderbook/MarketBook.hpp"

using namespace gridx::matching;
using namespace gridx::matching::orderbook;

class MarketBookTest : public ::testing::Test {
protected:
    MarketBook marketBook{MarketId{}};

    OrderPtr makeOrder(OrderId id, GridZoneId zone, Side side, Price price) {
        auto order = std::make_shared<Order>();

        order->orderId = id;
        order->userId = 1;

        order->marketId = {};
        order->gridZone = zone;

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

TEST_F(MarketBookTest, StoresMarketId) {
    EXPECT_EQ(marketBook.marketId(), MarketId{});
}

TEST_F(MarketBookTest, CreatesZoneBookWhenFirstOrderIsAdded) {
    marketBook.addOrder(makeOrder(1, 1, Side::Buy, 100));

    ASSERT_EQ(marketBook.zoneOrderBooks().size(), 1);

    auto it = marketBook.zoneOrderBooks().find(1);

    ASSERT_NE(it, marketBook.zoneOrderBooks().end());

    EXPECT_EQ(it->second.gridZone(), 1);
}

TEST_F(MarketBookTest, ReusesExistingZoneBook) {
    marketBook.addOrder(makeOrder(1, 1, Side::Buy, 100));

    marketBook.addOrder(makeOrder(2, 1, Side::Sell, 105));

    EXPECT_EQ(marketBook.zoneOrderBooks().size(), 1);

    auto it = marketBook.zoneOrderBooks().find(1);

    ASSERT_NE(it, marketBook.zoneOrderBooks().end());

    EXPECT_FALSE(it->second.buyBook().empty());
    EXPECT_FALSE(it->second.sellBook().empty());
}

TEST_F(MarketBookTest, CreatesSeparateZoneBooks) {
    marketBook.addOrder(makeOrder(1, 1, Side::Buy, 100));

    marketBook.addOrder(makeOrder(2, 2, Side::Buy, 100));

    marketBook.addOrder(makeOrder(3, 3, Side::Sell, 100));

    EXPECT_EQ(marketBook.zoneOrderBooks().size(), 3);
}

TEST_F(MarketBookTest, RoutesOrdersToCorrectZoneBook) {
    marketBook.addOrder(makeOrder(1, 1, Side::Buy, 100));

    marketBook.addOrder(makeOrder(2, 2, Side::Sell, 105));

    auto zone1 = marketBook.zoneOrderBooks().find(1);
    auto zone2 = marketBook.zoneOrderBooks().find(2);

    ASSERT_NE(zone1, marketBook.zoneOrderBooks().end());
    ASSERT_NE(zone2, marketBook.zoneOrderBooks().end());

    EXPECT_FALSE(zone1->second.buyBook().empty());
    EXPECT_TRUE(zone1->second.sellBook().empty());

    EXPECT_TRUE(zone2->second.buyBook().empty());
    EXPECT_FALSE(zone2->second.sellBook().empty());
}

TEST_F(MarketBookTest, ReturnsExistingZoneOrderBook) {
    marketBook.addOrder(makeOrder(1, 1, Side::Buy, 100));

    auto& zone1 = marketBook.zoneOrderBook(1);

    EXPECT_EQ(zone1.gridZone(), 1);

    EXPECT_FALSE(zone1.buyBook().empty());
}

TEST_F(MarketBookTest, CreatesZoneBookOnDemand) {
    auto& zone = marketBook.zoneOrderBook(5);

    EXPECT_EQ(zone.gridZone(), 5);

    EXPECT_TRUE(zone.buyBook().empty());
    EXPECT_TRUE(zone.sellBook().empty());

    EXPECT_EQ(marketBook.zoneOrderBooks().size(), 1);
}