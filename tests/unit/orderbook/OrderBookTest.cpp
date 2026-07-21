#include <gtest/gtest.h>

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
    EXPECT_TRUE(book.priceLevels().empty());
    EXPECT_EQ(book.bestPriceLevel(), nullptr);
}

TEST_F(BuyOrderBookTest, InsertSingleOrder) {
    auto order = makeOrder(1, 100);

    book.addOrder(order);

    EXPECT_FALSE(book.empty());

    ASSERT_EQ(book.priceLevels().size(), 1);

    auto it = book.priceLevels().find(100);

    ASSERT_NE(it, book.priceLevels().end());
    ASSERT_EQ(it->second.size(), 1);

    EXPECT_EQ(it->second.front()->orderId, 1);
}

TEST_F(BuyOrderBookTest, InsertMultipleOrdersSamePriceMaintainsFIFO) {
    book.addOrder(makeOrder(1, 100));
    book.addOrder(makeOrder(2, 100));
    book.addOrder(makeOrder(3, 100));

    const auto& queue = book.priceLevels().at(100);

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

    auto it = book.priceLevels().begin();

    ASSERT_NE(it, book.priceLevels().end());
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

    auto it = book.priceLevels().find(100);

    ASSERT_NE(it, book.priceLevels().end());

    ASSERT_EQ(it->second.size(), 1);

    EXPECT_EQ(it->second.front()->orderId, 2);
}

TEST_F(BuyOrderBookTest, RemoveLastOrderRemovesPriceLevel) {
    book.addOrder(makeOrder(1, 100));

    book.removeFrontOrder(100);

    EXPECT_TRUE(book.empty());
    EXPECT_TRUE(book.priceLevels().empty());
}

TEST_F(BuyOrderBookTest, ClearRemovesAllOrders) {
    book.addOrder(makeOrder(1, 100));
    book.addOrder(makeOrder(2, 105));
    book.addOrder(makeOrder(3, 95));

    book.clear();

    EXPECT_TRUE(book.empty());
    EXPECT_TRUE(book.priceLevels().empty());
    EXPECT_EQ(book.bestPriceLevel(), nullptr);
}

TEST_F(BuyOrderBookTest, BestPriceLevelReturnsHighestBid) {
    book.addOrder(makeOrder(1, 100));
    book.addOrder(makeOrder(2, 110));
    book.addOrder(makeOrder(3, 105));

    const OrderQueue* best = book.bestPriceLevel();

    ASSERT_NE(best, nullptr);
    ASSERT_EQ(best->size(), 1);

    EXPECT_EQ(best->front()->price, 110);
}

TEST_F(BuyOrderBookTest, RemoveUnknownPriceDoesNotModifyBook) {
    book.addOrder(makeOrder(1, 100));

    book.removeFrontOrder(999);

    ASSERT_EQ(book.priceLevels().size(), 1);

    auto it = book.priceLevels().find(100);

    ASSERT_NE(it, book.priceLevels().end());
    EXPECT_EQ(it->second.front()->orderId, 1);
}

TEST_F(SellOrderBookTest, MaintainsAscendingPriceOrder) {
    book.addOrder(makeOrder(1, 100));
    book.addOrder(makeOrder(2, 110));
    book.addOrder(makeOrder(3, 105));
    book.addOrder(makeOrder(4, 95));

    auto it = book.priceLevels().begin();

    ASSERT_NE(it, book.priceLevels().end());

    EXPECT_EQ(it->first, 95);

    ++it;
    EXPECT_EQ(it->first, 100);

    ++it;
    EXPECT_EQ(it->first, 105);

    ++it;
    EXPECT_EQ(it->first, 110);
}