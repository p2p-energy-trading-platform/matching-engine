#include <gtest/gtest.h>

#include <algorithm>
#include <functional>
#include <memory>

#include "gridx/matching/orderbook/OrderBook.hpp"

using namespace gridx::matching;
using namespace gridx::matching::orderbook;

class BuyOrderBookTest : public ::testing::Test {
protected:
    using BuyBook = OrderBook<std::greater<Price>>;

    BuyBook book;

    OrderPtr makeOrder(OrderId id, Price price) {
        auto order = std::make_shared<Order>();

        order->orderId = id;
        order->userId = 1;

        order->marketId = {};
        order->gridZone = 1;

        order->side = Side::Buy;
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

class SellOrderBookTest : public ::testing::Test {
protected:
    using SellBook = OrderBook<std::less<Price>>;

    SellBook book;

    OrderPtr makeOrder(OrderId id, Price price) {
        auto order = std::make_shared<Order>();

        order->orderId = id;
        order->userId = 1;

        order->marketId = {};
        order->gridZone = 1;

        order->side = Side::Sell;
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

TEST_F(BuyOrderBookTest, EmptyOrderBook) {
    EXPECT_TRUE(book.empty());
    EXPECT_TRUE(book.snapshotPriceLevels().empty());
    EXPECT_TRUE(book.snapshotOrders().empty());
}

TEST_F(BuyOrderBookTest, InsertSingleOrder) {
    auto order = makeOrder(1, 100);

    book.addOrder(order);

    EXPECT_FALSE(book.empty());

    const auto priceLevels = book.snapshotPriceLevels();
    ASSERT_EQ(priceLevels.size(), 1);

    auto it = std::find_if(priceLevels.begin(), priceLevels.end(),
                           [](const auto& entry) { return entry.first == 100; });

    ASSERT_NE(it, priceLevels.end());
    ASSERT_EQ(it->second.size(), 1);

    EXPECT_EQ(it->second.front()->orderId, 1);
}

TEST_F(BuyOrderBookTest, InsertMultipleOrdersSamePriceMaintainsFIFO) {
    book.addOrder(makeOrder(1, 100));
    book.addOrder(makeOrder(2, 100));
    book.addOrder(makeOrder(3, 100));

    const auto priceLevels = book.snapshotPriceLevels();
    auto it = std::find_if(priceLevels.begin(), priceLevels.end(),
                           [](const auto& entry) { return entry.first == 100; });

    ASSERT_NE(it, priceLevels.end());
    const auto& queue = it->second;

    ASSERT_EQ(queue.size(), 3);

    EXPECT_EQ(queue[0]->orderId, 1);
    EXPECT_EQ(queue[1]->orderId, 2);
    EXPECT_EQ(queue[2]->orderId, 3);
}

TEST_F(BuyOrderBookTest, MaintainsDescendingPriceOrder) {
    book.addOrder(makeOrder(1, 100));
    book.addOrder(makeOrder(2, 110));
    book.addOrder(makeOrder(3, 105));
    book.addOrder(makeOrder(4, 95));

    const auto priceLevels = book.snapshotPriceLevels();
    auto it = priceLevels.begin();

    ASSERT_NE(it, priceLevels.end());
    EXPECT_EQ(it->first, 110);

    ++it;
    EXPECT_EQ(it->first, 105);

    ++it;
    EXPECT_EQ(it->first, 100);

    ++it;
    EXPECT_EQ(it->first, 95);
}

TEST_F(BuyOrderBookTest, RemoveFrontOrderMaintainsFIFO) {
    book.addOrder(makeOrder(1, 100));
    book.addOrder(makeOrder(2, 100));

    book.removeFrontOrder(100);

    const auto priceLevels = book.snapshotPriceLevels();
    auto it = std::find_if(priceLevels.begin(), priceLevels.end(),
                           [](const auto& entry) { return entry.first == 100; });

    ASSERT_NE(it, priceLevels.end());

    ASSERT_EQ(it->second.size(), 1);

    EXPECT_EQ(it->second.front()->orderId, 2);
}

TEST_F(BuyOrderBookTest, RemoveLastOrderRemovesPriceLevel) {
    book.addOrder(makeOrder(1, 100));

    book.removeFrontOrder(100);

    EXPECT_TRUE(book.empty());
    EXPECT_TRUE(book.snapshotPriceLevels().empty());
}

TEST_F(BuyOrderBookTest, ClearRemovesAllOrders) {
    book.addOrder(makeOrder(1, 100));
    book.addOrder(makeOrder(2, 105));
    book.addOrder(makeOrder(3, 95));

    book.clear();

    EXPECT_TRUE(book.empty());
    EXPECT_TRUE(book.snapshotPriceLevels().empty());
    EXPECT_TRUE(book.snapshotOrders().empty());
}

TEST_F(BuyOrderBookTest, BestPriceLevelReturnsHighestBid) {
    book.addOrder(makeOrder(1, 100));
    book.addOrder(makeOrder(2, 110));
    book.addOrder(makeOrder(3, 105));

    const auto best = book.bestOrder();

    ASSERT_NE(best, nullptr);
    EXPECT_EQ(best->price, 110);
}

TEST_F(BuyOrderBookTest, RemoveUnknownPriceDoesNotModifyBook) {
    book.addOrder(makeOrder(1, 100));

    book.removeFrontOrder(999);

    const auto priceLevels = book.snapshotPriceLevels();
    ASSERT_EQ(priceLevels.size(), 1);

    auto it = std::find_if(priceLevels.begin(), priceLevels.end(),
                           [](const auto& entry) { return entry.first == 100; });

    ASSERT_NE(it, priceLevels.end());
    EXPECT_EQ(it->second.front()->orderId, 1);
}

TEST_F(SellOrderBookTest, MaintainsAscendingPriceOrder) {
    book.addOrder(makeOrder(1, 100));
    book.addOrder(makeOrder(2, 110));
    book.addOrder(makeOrder(3, 105));
    book.addOrder(makeOrder(4, 95));

    const auto priceLevels = book.snapshotPriceLevels();
    auto it = priceLevels.begin();

    ASSERT_NE(it, priceLevels.end());

    EXPECT_EQ(it->first, 95);

    ++it;
    EXPECT_EQ(it->first, 100);

    ++it;
    EXPECT_EQ(it->first, 105);

    ++it;
    EXPECT_EQ(it->first, 110);
}

TEST_F(BuyOrderBookTest, SnapshotOrdersWhenEmpty) {
    EXPECT_TRUE(book.snapshotOrders().empty());
}

TEST_F(BuyOrderBookTest, SnapshotOrdersReturnsSingleOrder) {
    book.addOrder(makeOrder(1, 100));

    const auto orders = book.snapshotOrders();

    ASSERT_EQ(orders.size(), 1);
    EXPECT_EQ(orders.front()->orderId, 1);
}

TEST_F(BuyOrderBookTest, SnapshotOrdersMaintainsFIFOAtSamePrice) {
    book.addOrder(makeOrder(1, 100));
    book.addOrder(makeOrder(2, 100));
    book.addOrder(makeOrder(3, 100));

    const auto orders = book.snapshotOrders();

    ASSERT_EQ(orders.size(), 3);

    EXPECT_EQ(orders[0]->orderId, 1);
    EXPECT_EQ(orders[1]->orderId, 2);
    EXPECT_EQ(orders[2]->orderId, 3);
}

TEST_F(BuyOrderBookTest, SnapshotOrdersMaintainsPriceTimePriority) {
    book.addOrder(makeOrder(1, 105));
    book.addOrder(makeOrder(2, 100));
    book.addOrder(makeOrder(3, 105));
    book.addOrder(makeOrder(4, 95));

    const auto orders = book.snapshotOrders();

    ASSERT_EQ(orders.size(), 4);

    EXPECT_EQ(orders[0]->orderId, 1);
    EXPECT_EQ(orders[1]->orderId, 3);
    EXPECT_EQ(orders[2]->orderId, 2);
    EXPECT_EQ(orders[3]->orderId, 4);
}

TEST_F(SellOrderBookTest, SnapshotOrdersMaintainsPriceTimePriority) {
    book.addOrder(makeOrder(1, 100));
    book.addOrder(makeOrder(2, 95));
    book.addOrder(makeOrder(3, 95));
    book.addOrder(makeOrder(4, 105));

    const auto orders = book.snapshotOrders();

    ASSERT_EQ(orders.size(), 4);

    EXPECT_EQ(orders[0]->orderId, 2);
    EXPECT_EQ(orders[1]->orderId, 3);
    EXPECT_EQ(orders[2]->orderId, 1);
    EXPECT_EQ(orders[3]->orderId, 4);
}