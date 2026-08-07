#pragma once

#include "gridx/matching/common/Types.hpp"

namespace gridx::matching {

struct Trade {
    TradeId tradeId;

    OrderId buyOrderId;
    OrderId sellOrderId;

    UserId buyerId;
    UserId sellerId;

    GridZoneId buyerGridZone;
    GridZoneId sellerGridZone;

    Timestamp deliverySlotStart;
    Timestamp deliverySlotEnd;

    Price energyPrice;
    GridFee gridFee;
    Price buyerTotalPrice;

    Quantity quantity;

    // Version of the grid transfer rules applied when this trade was executed.
    GridRuleVersion gridRuleVersion;

    Timestamp timestamp;
};

}  // namespace gridx::matching