#pragma once

#include "gridx/matching/domain/OrderUpdate.hpp"

#include "gridx/order/v1/order_update_events.pb.h"

namespace gridx::matching::adapters::kafka {

class OrderUpdateEventMapper {
public:
    /**
     * Converts a domain OrderUpdate into a protobuf OrderUpdated event.
     */
    [[nodiscard]]
    gridx::order::v1::OrderUpdated toProtobuf(const OrderUpdate& update) const;

private:
    /**
     * Converts a domain OrderStatus to its protobuf equivalent.
     */
    [[nodiscard]]
    static gridx::order::v1::OrderStatus toProtobufOrderStatus(OrderStatus status);

    /**
     * Converts a domain OrderUpdateAction to its protobuf event type.
     */
    [[nodiscard]]
    static gridx::order::v1::OrderUpdateType toProtobufUpdateType(OrderUpdateAction action,
                                                                  OrderStatus status);
};

}  // namespace gridx::matching::adapters::kafka