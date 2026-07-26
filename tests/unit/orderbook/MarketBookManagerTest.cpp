#include <gtest/gtest.h>

#include "gridx/matching/orderbook/MarketBookManager.hpp"

using namespace gridx::matching;
using namespace gridx::matching::orderbook;

class MarketBookManagerTest : public ::testing::Test {
protected:
    MarketBookManager manager;

    MarketId makeMarket(std::int64_t slot) {
        MarketId marketId;
        marketId.deliverySlotStart = Timestamp{std::chrono::seconds{slot}};
        return marketId;
    }
};

TEST_F(MarketBookManagerTest, InitiallyEmpty) {
    EXPECT_TRUE(manager.empty());
    EXPECT_EQ(manager.size(), 0);
}

TEST_F(MarketBookManagerTest, CreatesMarketBookWhenMissing) {
    auto marketId = makeMarket(0);

    auto& marketBook = manager.getOrCreate(marketId);

    EXPECT_EQ(manager.size(), 1);
    EXPECT_FALSE(manager.empty());

    EXPECT_EQ(marketBook.marketId(), marketId);
}

TEST_F(MarketBookManagerTest, ReturnsExistingMarketBook) {
    auto marketId = makeMarket(0);

    auto& first = manager.getOrCreate(marketId);
    auto& second = manager.getOrCreate(marketId);

    EXPECT_EQ(&first, &second);
    EXPECT_EQ(manager.size(), 1);
}

TEST_F(MarketBookManagerTest, CreatesDifferentMarketBooksForDifferentMarkets) {
    auto market1 = makeMarket(0);
    auto market2 = makeMarket(1800);

    auto& first = manager.getOrCreate(market1);
    auto& second = manager.getOrCreate(market2);

    EXPECT_NE(&first, &second);
    EXPECT_EQ(manager.size(), 2);
}

TEST_F(MarketBookManagerTest, FindReturnsExistingMarketBook) {
    auto marketId = makeMarket(0);

    manager.getOrCreate(marketId);

    auto* marketBook = manager.find(marketId);

    ASSERT_NE(marketBook, nullptr);
    EXPECT_EQ(marketBook->marketId(), marketId);
}

TEST_F(MarketBookManagerTest, FindReturnsNullptrWhenMarketBookDoesNotExist) {
    auto marketId = makeMarket(0);

    EXPECT_EQ(manager.find(marketId), nullptr);
}

TEST_F(MarketBookManagerTest, ClearRemovesAllMarketBooks) {
    manager.getOrCreate(makeMarket(0));
    manager.getOrCreate(makeMarket(1800));
    manager.getOrCreate(makeMarket(3600));

    ASSERT_EQ(manager.size(), 3);

    manager.clear();

    EXPECT_TRUE(manager.empty());
    EXPECT_EQ(manager.size(), 0);
}

TEST_F(MarketBookManagerTest, ConstFindReturnsExistingMarketBook) {
    auto marketId = makeMarket(0);

    manager.getOrCreate(marketId);

    const MarketBookManager& constManager = manager;

    auto* marketBook = constManager.find(marketId);

    ASSERT_NE(marketBook, nullptr);
    EXPECT_EQ(marketBook->marketId(), marketId);
}