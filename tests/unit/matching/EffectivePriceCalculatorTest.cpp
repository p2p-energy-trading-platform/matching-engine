#include <gtest/gtest.h>
#include "gridx/matching/common/Types.hpp"
#include "gridx/matching/config/GridTransferCache.hpp"
#include "gridx/matching/domain/GridTransferRule.hpp"
#include "gridx/matching/matching/EffectivePriceCalculator.hpp"

namespace {

using gridx::matching::GridFee;
using gridx::matching::GridTransferRule;
using gridx::matching::GridZoneId;
using gridx::matching::Price;
using gridx::matching::matching::EffectivePriceCalculator;

class EffectivePriceCalculatorTest : public ::testing::Test {
protected:
    const GridZoneId zoneA{1};
    const GridZoneId zoneB{2};
    const GridZoneId zoneC{3};

    gridx::matching::config::GridTransferCache cache;

    void SetUp() override {
        // Setup a valid/allowed cross-zone rule from Zone A to Zone B (Fee = 5)
        GridTransferRule allowedRule{};
        allowedRule.sellerGridZone = zoneA;
        allowedRule.buyerGridZone = zoneB;
        allowedRule.allowed = true;
        allowedRule.gridFeePerKwh = 5;
        cache.upsert(allowedRule);

        // Setup a disallowed cross-zone rule from Zone A to Zone C
        GridTransferRule disallowedRule{};
        disallowedRule.sellerGridZone = zoneA;
        disallowedRule.buyerGridZone = zoneC;
        disallowedRule.allowed = false;
        disallowedRule.gridFeePerKwh = 10;
        cache.upsert(disallowedRule);
    }
};

TEST_F(EffectivePriceCalculatorTest, PureCalculateEffectiveAsk) {
    const Price sellerPrice = 45;
    const GridFee fee = 4;

    const Price effectiveAsk = EffectivePriceCalculator::calculateEffectiveAsk(sellerPrice, fee);

    // effective_ask = seller_price + grid_fee
    EXPECT_EQ(effectiveAsk, 49);
}

TEST_F(EffectivePriceCalculatorTest, PureCalculateEffectiveBid) {
    const Price buyerPrice = 50;
    const GridFee fee = 4;

    const Price effectiveBid = EffectivePriceCalculator::calculateEffectiveBid(buyerPrice, fee);

    // effective_bid = buyer_limit_price - grid_fee
    EXPECT_EQ(effectiveBid, 46);
}

TEST_F(EffectivePriceCalculatorTest, EffectiveAskSameZone) {
    const Price sellerPrice = 100;

    // Same zone matching (sellerZone == buyerZone) -> Grid fee is 0
    auto result = EffectivePriceCalculator::calculateEffectiveAsk(sellerPrice, zoneA, zoneA, cache);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 100);
}

TEST_F(EffectivePriceCalculatorTest, EffectiveAskCrossZoneAllowed) {
    const Price sellerPrice = 45;

    // Cross-zone (Zone A -> Zone B) with fee = 5
    auto result = EffectivePriceCalculator::calculateEffectiveAsk(sellerPrice, zoneA, zoneB, cache);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 50);  // 45 + 5
}

TEST_F(EffectivePriceCalculatorTest, EffectiveAskCrossZoneNotAllowed) {
    const Price sellerPrice = 45;

    // Cross-zone (Zone A -> Zone C) is explicitly not allowed
    auto result = EffectivePriceCalculator::calculateEffectiveAsk(sellerPrice, zoneA, zoneC, cache);

    EXPECT_FALSE(result.has_value());
}

TEST_F(EffectivePriceCalculatorTest, EffectiveAskCrossZoneMissingRule) {
    const Price sellerPrice = 45;
    const GridZoneId unknownZone{99};

    // Rule does not exist in cache
    auto result =
        EffectivePriceCalculator::calculateEffectiveAsk(sellerPrice, zoneA, unknownZone, cache);

    EXPECT_FALSE(result.has_value());
}

TEST_F(EffectivePriceCalculatorTest, EffectiveBidSameZone) {
    const Price buyerPrice = 100;

    // Same zone matching (sellerZone == buyerZone) -> Grid fee is 0
    auto result = EffectivePriceCalculator::calculateEffectiveBid(buyerPrice, zoneA, zoneA, cache);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 100);
}

TEST_F(EffectivePriceCalculatorTest, EffectiveBidCrossZoneAllowed) {
    const Price buyerPrice = 50;

    // Cross-zone (Zone A -> Zone B) with fee = 5
    auto result = EffectivePriceCalculator::calculateEffectiveBid(buyerPrice, zoneA, zoneB, cache);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 45);  // 50 - 5
}

TEST_F(EffectivePriceCalculatorTest, EffectiveBidCrossZoneNotAllowed) {
    const Price buyerPrice = 50;

    // Cross-zone (Zone A -> Zone C) is explicitly not allowed
    auto result = EffectivePriceCalculator::calculateEffectiveBid(buyerPrice, zoneA, zoneC, cache);

    EXPECT_FALSE(result.has_value());
}

TEST_F(EffectivePriceCalculatorTest, EffectiveBidCrossZoneMissingRule) {
    const Price buyerPrice = 50;
    const GridZoneId unknownZone{99};

    // Rule does not exist in cache
    auto result =
        EffectivePriceCalculator::calculateEffectiveBid(buyerPrice, zoneA, unknownZone, cache);

    EXPECT_FALSE(result.has_value());
}

}  // namespace