#include <gtest/gtest.h>

#include "gridx/matching/orderbook/MarketBook.hpp"
#include "support/TestSupport.hpp"

using namespace gridx::matching;
using namespace gridx::matching::orderbook;
using namespace gridx::matching::test_support;

class MarketBookTest : public ::testing::Test {
protected:
    MarketBook marketBook{MarketId{}};

    OrderPtr makeOrder(OrderId id, GridZoneId zone, Side side, Price price) {
        return OrderBuilder{}
            .withOrderId(id)
            .withUserId(1)
            .withMarketId(MarketId{})
            .withGridZone(zone)
            .withSide(side)
            .withOrderType(OrderType::Limit)
            .withStatus(OrderStatus::New)
            .withPrice(price)
            .withQuantity(100)
            .withCreatedAt(Timestamp{})
            .withExpiresAt(Timestamp{})
            .buildPtr();
    }
};

TEST_F(MarketBookTest, StoresMarketId) {
    EXPECT_EQ(marketBook.marketId(), MarketId{});
}

TEST_F(MarketBookTest, CreatesZoneBookWhenFirstOrderIsAdded) {
    marketBook.addOrder(makeOrder(1, 1, Side::Buy, 100));

    ASSERT_EQ(marketBook.zoneOrderBookCount(), 1);

    const auto* zone = marketBook.findZoneOrderBook(1);

    ASSERT_NE(zone, nullptr);

    EXPECT_EQ(zone->gridZone(), 1);
}

TEST_F(MarketBookTest, ReusesExistingZoneBook) {
    marketBook.addOrder(makeOrder(1, 1, Side::Buy, 100));

    marketBook.addOrder(makeOrder(2, 1, Side::Sell, 105));

    EXPECT_EQ(marketBook.zoneOrderBookCount(), 1);

    const auto* zone = marketBook.findZoneOrderBook(1);

    ASSERT_NE(zone, nullptr);

    EXPECT_FALSE(zone->empty(Side::Buy));
    EXPECT_FALSE(zone->empty(Side::Sell));
}

TEST_F(MarketBookTest, CreatesSeparateZoneBooks) {
    marketBook.addOrder(makeOrder(1, 1, Side::Buy, 100));

    marketBook.addOrder(makeOrder(2, 2, Side::Buy, 100));

    marketBook.addOrder(makeOrder(3, 3, Side::Sell, 100));

    EXPECT_EQ(marketBook.zoneOrderBookCount(), 3);
}

TEST_F(MarketBookTest, RoutesOrdersToCorrectZoneBook) {
    marketBook.addOrder(makeOrder(1, 1, Side::Buy, 100));

    marketBook.addOrder(makeOrder(2, 2, Side::Sell, 105));

    const auto* zone1 = marketBook.findZoneOrderBook(1);
    const auto* zone2 = marketBook.findZoneOrderBook(2);

    ASSERT_NE(zone1, nullptr);
    ASSERT_NE(zone2, nullptr);

    EXPECT_FALSE(zone1->empty(Side::Buy));
    EXPECT_TRUE(zone1->empty(Side::Sell));

    EXPECT_TRUE(zone2->empty(Side::Buy));
    EXPECT_FALSE(zone2->empty(Side::Sell));
}

TEST_F(MarketBookTest, ReturnsExistingZoneOrderBook) {
    marketBook.addOrder(makeOrder(1, 1, Side::Buy, 100));

    auto& zone1 = marketBook.zoneOrderBook(1);

    EXPECT_EQ(zone1.gridZone(), 1);

    EXPECT_FALSE(zone1.empty(Side::Buy));
}

TEST_F(MarketBookTest, CreatesZoneBookOnDemand) {
    auto& zone = marketBook.zoneOrderBook(5);

    EXPECT_EQ(zone.gridZone(), 5);

    EXPECT_TRUE(zone.empty(Side::Buy));
    EXPECT_TRUE(zone.empty(Side::Sell));

    EXPECT_EQ(marketBook.zoneOrderBookCount(), 1);
}