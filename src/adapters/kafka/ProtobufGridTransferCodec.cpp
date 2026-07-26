#include <optional>
#include "gridx/grid/v1/grid_transfer_rule.pb.h"
namespace gridx::matching::adapters::kafka {

std::optional<gridx::grid::v1::GridTransferRule> deserialize(
    const std::span<const std::byte> payload) {
    if (payload.empty()) {
        return std::nullopt;
    }

    gridx::grid::v1::GridTransferRule transferRule;

    // const auto data = reinterpret_cast<const void*>(payload.data());

    const bool parsed =
        transferRule.ParseFromArray(payload.data(), static_cast<int>(payload.size()));

    if (!parsed) {
        return std::nullopt;
    }

    return transferRule;
};

}  // namespace gridx::matching::adapters::kafka