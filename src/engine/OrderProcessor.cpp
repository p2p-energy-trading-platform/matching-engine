#include "gridx/matching/engine/OrderProcessor.hpp"

#include <utility>

namespace gridx::matching::engine {

OrderProcessor::OrderProcessor(adapters::kafka::OrderEventMapper mapper,
                               adapters::kafka::ProtobufOrderCodec codec,
                               validation::OrderValidator& validator)
    : mapper_{std::move(mapper)}, codec_{std::move(codec)}, validator_{validator} {}

void OrderProcessor::process(const std::span<const std::byte> payload) {
    const auto event = codec_.deserialize(payload);
    const auto order = mapper_.toDomain(event);

    validator_.validate(order);

    // TODO: submit order in matching pipeline
}

}  // namespace gridx::matching::engine