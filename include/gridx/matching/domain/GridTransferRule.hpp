#pragma once

#include "gridx/matching/common/Types.hpp"

namespace gridx::matching {

struct GridTransferRule {
    GridZoneId sellerGridZone{};
    GridZoneId buyerGridZone{};

    bool allowed{0};

    GridFee gridFeePerKwh{0};

    GridRuleVersion version{0};
    Timestamp updatedAt{};
};

}  // namespace gridx::matching