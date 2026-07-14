#pragma once

#include "gridx/matching/common/Types.hpp"

namespace gridx::matching
{

struct Trade
{
    TradeId tradeId;

    OrderId buyOrderId;
    OrderId sellOrderId;

    UserId buyerId;
    UserId sellerId;

    GridZoneId buyerGridZone;
    GridZoneId sellerGridZone;

    Price energyPrice;
    GridFee gridFee;

    Quantity quantity;

    std::uint64_t gridRuleVersion;

    Timestamp timestamp;
};

} // namespace gridx::matching