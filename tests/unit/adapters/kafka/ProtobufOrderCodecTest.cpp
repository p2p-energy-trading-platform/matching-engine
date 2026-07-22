#include <gtest/gtest.h>

#include "gridx/matching/adapters/kafka/ProtobufOrderCodec.hpp"
#include "gridx/order/v1/order_events.pb.h"

namespace {
using gridx::matching::adapters::kafka::ProtobufOrderCodec;
using gridx::order::v1::OrderAccepted;
}  // namespace

TEST(ProtobufOrderCodecTest, DeserializesValidOrderAccepted) {
    OrderAccepted original;

    original.set_order_id(101);
    original.set_user_id(202);
    original.set_grid_zone_id(3);
    original.set_side(gridx::order::v1::SIDE_BUY);
    original.set_order_type(gridx::order::v1::ORDER_TYPE_LIMIT);
    original.set_status(gridx::order::v1::ORDER_STATUS_NEW);
    original.set_price(5000);
    original.set_quantity(100);

    const std::string serialized = original.SerializeAsString();

    const auto characters = std::span{serialized.data(), serialized.size()};

    const auto payload = std::as_bytes(characters);

    ProtobufOrderCodec codec;

    const auto decoded = codec.deserialize(payload);

    EXPECT_EQ(decoded.order_id(), original.order_id());
    EXPECT_EQ(decoded.user_id(), original.user_id());
    EXPECT_EQ(decoded.side(), original.side());
    EXPECT_EQ(decoded.price(), original.price());
}

TEST(ProtobufOrderCodecTest, RejectsEmptyPayload) {
    ProtobufOrderCodec codec;
    const std::span<const std::byte> payload;

    EXPECT_THROW(static_cast<void>(codec.deserialize(payload)), std::invalid_argument);
}

TEST(ProtobufOrderCodecTest, RejectsMalformedPayload) {
    ProtobufOrderCodec codec;

    const std::array payload{std::byte{0xFF}};

    EXPECT_THROW(static_cast<void>(codec.deserialize(payload)), std::runtime_error);
}