#include "gridx/matching/engine/OrderProcessor.hpp"

#include <spdlog/spdlog.h>

#include "gridx/matching/adapters/kafka/ProtobufCodec.hpp"
#include "gridx/order/v1/order_events.pb.h"

namespace gridx::matching::engine {

OrderProcessor::OrderProcessor(adapters::kafka::OrderEventMapper& orderEventMapper,
                               validation::OrderValidator& validator,
                               matching::MatchingEngine& matchingEngine,
                               orderbook::OrderBookUpdater& orderBookUpdater,
                               adapters::kafka::MatchingResultPublisher& matchingResultPublisher)
    : m_orderEventMapper(orderEventMapper)
    , m_validator(validator)
    , m_matchingEngine(matchingEngine)
    , m_orderBookUpdater(orderBookUpdater)
    , m_matchingResultPublisher(matchingResultPublisher) {}

void OrderProcessor::process(std::span<const std::byte> payload) {
    const auto event =
        adapters::kafka::ProtobufCodec::deserialize<gridx::order::v1::OrderAccepted>(payload);

    if (!event) {
        spdlog::debug("Received invalid order payload");
        return;
    }

    const Order order = m_orderEventMapper.toDomain(*event);

    m_validator.validate(order);

    const MatchingResult result = m_matchingEngine.match(order);

    // TODO:
    // Apply MatchingResult to MarketBook.

    m_matchingResultPublisher.publish(result);
}

}  // namespace gridx::matching::engine