#include "gridx/matching/adapters/kafka/OrderUpdateEventMapper.hpp"

#include <chrono>
#include <stdexcept>

namespace gridx::matching::adapters::kafka {

namespace {

google::protobuf::Timestamp toProtobufTimestamp(const Timestamp& timestamp) {
    using namespace std::chrono;

    const auto duration = timestamp.time_since_epoch();

    const auto seconds = duration_cast<std::chrono::seconds>(duration);

    const auto nanos =
        duration_cast<std::chrono::nanoseconds>(duration - seconds);

    google::protobuf::Timestamp protoTimestamp;

    protoTimestamp.set_seconds(seconds.count());
    protoTimestamp.set_nanos(static_cast<std::int32_t>(nanos.count()));

    return protoTimestamp;
}

}  // namespace

gridx::order::v1::OrderUpdated
OrderUpdateEventMapper::toProtobuf(const OrderUpdate& update) const {
    if (!update.order) {
        throw std::invalid_argument("Order update does not contain an order");
    }

    gridx::order::v1::OrderUpdated event;

    // TODO: Replace with a proper Event ID generator.
    event.set_event_id("");

    event.set_event_type(
        toProtobufUpdateType(update.action, update.status));

    event.set_order_id(update.order->orderId);

    event.set_status(toProtobufOrderStatus(update.status));

    event.set_remaining_quantity(update.remainingQuantity);

    *event.mutable_updated_at() =
        toProtobufTimestamp(std::chrono::system_clock::now());

    return event;
}

gridx::order::v1::OrderStatus
OrderUpdateEventMapper::toProtobufOrderStatus(OrderStatus status) {
    using ProtoStatus = gridx::order::v1::OrderStatus;

    switch (status) {
        case OrderStatus::New:
            return ProtoStatus::ORDER_STATUS_NEW;

        case OrderStatus::PartiallyFilled:
            return ProtoStatus::ORDER_STATUS_PARTIALLY_FILLED;

        case OrderStatus::Filled:
            return ProtoStatus::ORDER_STATUS_FILLED;

        case OrderStatus::Cancelled:
            return ProtoStatus::ORDER_STATUS_CANCELLED;

        case OrderStatus::Expired:
            return ProtoStatus::ORDER_STATUS_EXPIRED;

        default:
            throw std::invalid_argument("Unsupported order status");
    }
}

gridx::order::v1::OrderUpdateType
OrderUpdateEventMapper::toProtobufUpdateType(OrderUpdateAction action,
                                             OrderStatus status) {
    using ProtoType = gridx::order::v1::OrderUpdateType;

    switch (action) {
        case OrderUpdateAction::Update:
            return status == OrderStatus::PartiallyFilled
                       ? ProtoType::ORDER_UPDATE_TYPE_PARTIALLY_FILLED
                       : ProtoType::ORDER_UPDATE_TYPE_UPDATED;

        case OrderUpdateAction::Remove:
            switch (status) {
                case OrderStatus::Filled:
                    return ProtoType::ORDER_UPDATE_TYPE_FILLED;

                case OrderStatus::Cancelled:
                    return ProtoType::ORDER_UPDATE_TYPE_CANCELLED;

                case OrderStatus::Expired:
                    return ProtoType::ORDER_UPDATE_TYPE_EXPIRED;

                default:
                    throw std::invalid_argument(
                        "Unsupported order update status");
            }

        default:
            throw std::invalid_argument("Unsupported order update action");
    }
}

}  // namespace gridx::matching::adapters::kafka