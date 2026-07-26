#pragma once

#include "gridx/grid/v1/grid_transfer_rule.pb.h"
#include "gridx/matching/domain/GridTransferRule.hpp"

namespace gridx::matching::adapters::kafka {

class GridTransferEventMapper {
public:
    [[nodiscard]]
    static std::optional<GridTransferRule> toDomain(
        const gridx::grid::v1::GridTransferRule& source);
};

}  // namespace gridx::matching::adapters::kafka