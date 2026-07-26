#pragma once

#include <cstddef>
#include <optional>
#include "gridx/grid/v1/grid_transfer_rule.pb.h"

namespace gridx::matching::adapters::kafka {

class ProtobufGridTransferCodec {
public:
    [[nodiscard]]
    static std::optional<gridx::grid::v1::GridTransferRule> deserialize(
        const std::span<const std::byte> payload);
};

}  // namespace gridx::matching::adapters::kafka