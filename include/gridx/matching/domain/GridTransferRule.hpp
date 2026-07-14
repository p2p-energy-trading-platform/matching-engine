#pragma once

#include "gridx/matching/common/Types.hpp"

namespace gridx::matching {

struct GridTransferRule {
    GridZoneId sellerGridZone;
    GridZoneId buyerGridZone;

    bool allowed;

    GridFee gridFee;

    GridRuleVersion version;
};

}  // namespace gridx::matching