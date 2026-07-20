#pragma once

#include "gridx/matching/domain/Order.hpp"

#include "gridx/order/v1/order_events.pb.h"

namespace gridx::matching::adapters::kafka {

class OrderEventMapper {
public:
    [[nodiscard]]
    Order toDomain(const gridx::order::v1::OrderAccepted& event) const;
};

}  // namespace gridx::matching::adapters::kafka