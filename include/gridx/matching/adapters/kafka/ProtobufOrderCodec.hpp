#pragma once

#include <cstddef>
#include <span>

#include <gridx/order/v1/order_events.pb.h>

namespace gridx::matching::adapters::kafka {

class ProtobufOrderCodec {
public:
    [[nodiscard]]
    gridx::order::v1::OrderAccepted deserialize(
        const std::span<const std::byte> payload
    ) const;
};

} // namespace gridx::matching::adapters::kafka