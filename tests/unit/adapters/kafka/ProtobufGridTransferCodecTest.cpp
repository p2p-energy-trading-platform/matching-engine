#include <gtest/gtest.h>

#include "gridx/grid/v1/grid_transfer_rule.pb.h"
#include "gridx/matching/adapters/kafka/ProtobufCodec.hpp"

namespace {

namespace proto = gridx::grid::v1;

using gridx::matching::adapters::kafka::ProtobufCodec;

class ProtobufGridTransferCodecTest : public ::testing::Test {
protected:
    static proto::GridTransferRule makeValidRule() {
        proto::GridTransferRule rule;

        rule.set_allowed(true);
        rule.set_grid_fee_per_kwh(25);
        rule.set_version(7);
        rule.set_seller_grid_zone_id(11);
        rule.set_buyer_grid_zone_id(12);

        auto* updatedAt = rule.mutable_updated_at();
        updatedAt->set_seconds(1'700'000'000);
        updatedAt->set_nanos(456'000'000);

        return rule;
    }
};

}  // namespace

TEST_F(ProtobufGridTransferCodecTest, DeserializesValidGridTransferRule) {
    const auto original = makeValidRule();

    const std::string serialized = original.SerializeAsString();
    const auto characters = std::span{serialized.data(), serialized.size()};
    const auto payload = std::as_bytes(characters);

    const auto decoded = ProtobufCodec::deserialize<proto::GridTransferRule>(payload);

    ASSERT_TRUE(decoded.has_value());
    EXPECT_EQ(decoded->allowed(), original.allowed());
    EXPECT_EQ(decoded->grid_fee_per_kwh(), original.grid_fee_per_kwh());
    EXPECT_EQ(decoded->version(), original.version());
    EXPECT_EQ(decoded->seller_grid_zone_id(), original.seller_grid_zone_id());
    EXPECT_EQ(decoded->buyer_grid_zone_id(), original.buyer_grid_zone_id());
    EXPECT_EQ(decoded->updated_at().seconds(), original.updated_at().seconds());
    EXPECT_EQ(decoded->updated_at().nanos(), original.updated_at().nanos());
}

TEST_F(ProtobufGridTransferCodecTest, RejectsEmptyPayload) {
    const std::span<const std::byte> payload;

    EXPECT_EQ(ProtobufCodec::deserialize<proto::GridTransferRule>(payload), std::nullopt);
}

TEST_F(ProtobufGridTransferCodecTest, RejectsMalformedPayload) {
    const std::array payload{std::byte{0xFF}};

    EXPECT_EQ(ProtobufCodec::deserialize<proto::GridTransferRule>(payload), std::nullopt);
}