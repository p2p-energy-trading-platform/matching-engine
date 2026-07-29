#include "gridx/matching/engine/OrderProcessor.hpp"

#include <utility>
#include "gridx/matching/adapters/kafka/ProtobufCodec.hpp"
#include "gridx/order/v1/order_events.pb.h"
#include <spdlog/spdlog.h>

namespace gridx::matching::engine {

OrderProcessor::OrderProcessor(adapters::kafka::OrderEventMapper mapper,
                               validation::OrderValidator& validator)
    : mapper_{std::move(mapper)}, validator_{validator} {}

void OrderProcessor::process(const std::span<const std::byte> payload) {
    const auto event = adapters::kafka::ProtobufCodec::deserialize<order::v1::OrderAccepted>(payload);

    if (!event) {
        spdlog::debug("Invalid order payload");
        return;
    }

    const auto order = mapper_.toDomain(*event);

    validator_.validate(order);

    // TODO: submit order in matching pipeline
}

}  // namespace gridx::matching::engine