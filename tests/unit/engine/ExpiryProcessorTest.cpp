#include <chrono>
#include <vector>

#include <gtest/gtest.h>

#include "gridx/matching/adapters/ports/IMatchingResultPublisher.hpp"
#include "gridx/matching/engine/ExpiryProcessor.hpp"
#include "support/TestSupport.hpp"

using namespace gridx::matching;
using namespace gridx::matching::engine;
using namespace gridx::matching::expiry;
using namespace gridx::matching::orderbook;
using namespace gridx::matching::test_support;

namespace {

class FakePublisher final : public adapters::ports::IMatchingResultPublisher {
public:
    void publish(const MatchingResult& result) override {
        published.push_back(result);
    }

    std::vector<MatchingResult> published;
};

class ExpiryProcessorTest : public ::testing::Test {
protected:
    ExpiryProcessorTest() : processor(marketBookManager, expiryManager, publisher) {}

    OrderPtr addOrder(const MarketId& marketId, OrderId orderId, std::uint64_t userId, Side side) {
        auto order = OrderBuilder{}
                         .withOrderId(orderId)
                         .withUserId(testUserId(userId))
                         .withMarketId(marketId)
                         .withSide(side)
                         .buildPtr();

        marketBookManager.getOrCreate(marketId).addOrder(order);

        return order;
    }

    const Timestamp now = std::chrono::system_clock::now();

    MarketBookManager marketBookManager;
    const ExpiryManager expiryManager;
    FakePublisher publisher;
    ExpiryProcessor processor;
};

}  // namespace

TEST_F(ExpiryProcessorTest, PublishesNothingWhenNoMarketHasEnded) {
    addOrder(MarketId{now}, 1, 10, Side::Buy);

    EXPECT_EQ(processor.process(now), 0U);
    EXPECT_TRUE(publisher.published.empty());
}

TEST_F(ExpiryProcessorTest, PublishesExpiredUpdatesForEndedMarket) {
    const MarketId endedMarket{now - std::chrono::hours{1}};

    auto buyOrder = addOrder(endedMarket, 1, 10, Side::Buy);
    auto sellOrder = addOrder(endedMarket, 2, 20, Side::Sell);

    EXPECT_EQ(processor.process(now), 2U);

    ASSERT_EQ(publisher.published.size(), 1U);

    const auto& result = publisher.published.front();

    EXPECT_TRUE(result.trades.empty());
    ASSERT_EQ(result.orderUpdates.size(), 2U);

    for (const auto& update : result.orderUpdates) {
        EXPECT_EQ(update.status, OrderStatus::Expired);
        EXPECT_EQ(update.action, OrderUpdateAction::Remove);
    }

    // The order (and therefore its owner) travels with each update.
    EXPECT_EQ(result.orderUpdates[0].order->userId, testUserId(10));
    EXPECT_EQ(result.orderUpdates[1].order->userId, testUserId(20));

    EXPECT_EQ(buyOrder->status, OrderStatus::Expired);
    EXPECT_EQ(sellOrder->status, OrderStatus::Expired);
}

TEST_F(ExpiryProcessorTest, DoesNotPublishTheSameOrdersTwice) {
    addOrder(MarketId{now - std::chrono::hours{1}}, 1, 10, Side::Buy);

    EXPECT_EQ(processor.process(now), 1U);
    EXPECT_EQ(processor.process(now), 0U);

    EXPECT_EQ(publisher.published.size(), 1U);
    EXPECT_TRUE(marketBookManager.empty());
}
