#include <gtest/gtest.h>

#include <chrono>

#include "gridx/grid/v1/grid_transfer_rule.pb.h"
#include "gridx/matching/adapters/kafka/GridTransferEventMapper.hpp"

namespace {

namespace proto = gridx::grid::v1;

using gridx::matching::GridTransferRule;
using gridx::matching::Timestamp;
using gridx::matching::adapters::kafka::GridTransferEventMapper;

class GridTransferEventMapperTest : public ::testing::Test {
protected:
    GridTransferEventMapper mapper;

    static proto::GridTransferRule makeValidEvent() {
        proto::GridTransferRule event;

        event.set_allowed(true);
        event.set_grid_fee_per_kwh(25);
        event.set_version(7);
        event.set_seller_grid_zone_id(11);
        event.set_buyer_grid_zone_id(12);

        auto* updatedAt = event.mutable_updated_at();
        updatedAt->set_seconds(1'700'000'000);
        updatedAt->set_nanos(456'000'000);

        return event;
    }

    static Timestamp expectedTimestamp(std::int64_t seconds, std::int32_t nanoseconds) {
        using namespace std::chrono;

        const auto duration = std::chrono::seconds{seconds} + std::chrono::nanoseconds{nanoseconds};

        return std::chrono::system_clock::time_point{
            duration_cast<std::chrono::system_clock::duration>(duration)};
    }
};

}  // namespace

TEST_F(GridTransferEventMapperTest, MapsValidEventToDomainRule) {
    const auto event = makeValidEvent();

    const auto rule = mapper.toDomain(event);

    ASSERT_TRUE(rule.has_value());
    EXPECT_EQ(rule->sellerGridZone, 11);
    EXPECT_EQ(rule->buyerGridZone, 12);
    EXPECT_EQ(rule->allowed, true);
    EXPECT_EQ(rule->gridFeePerKwh, 25);
    EXPECT_EQ(rule->version, 7);
    EXPECT_EQ(rule->updatedAt, expectedTimestamp(1'700'000'000, 456'000'000));
}

TEST_F(GridTransferEventMapperTest, RejectsMissingUpdatedAt) {
    auto event = makeValidEvent();
    event.clear_updated_at();

    EXPECT_EQ(mapper.toDomain(event), std::nullopt);
}

TEST_F(GridTransferEventMapperTest, RejectsZeroSellerZone) {
    auto event = makeValidEvent();
    event.set_seller_grid_zone_id(0);

    EXPECT_EQ(mapper.toDomain(event), std::nullopt);
}

TEST_F(GridTransferEventMapperTest, RejectsNegativeGridFee) {
    auto event = makeValidEvent();
    event.set_grid_fee_per_kwh(-1);

    EXPECT_EQ(mapper.toDomain(event), std::nullopt);
}