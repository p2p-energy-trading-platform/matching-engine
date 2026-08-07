#pragma once

#include <span>

#include "gridx/matching/adapters/kafka/MatchingResultPublisher.hpp"
#include "gridx/matching/adapters/kafka/OrderEventMapper.hpp"
#include "gridx/matching/matching/MatchingEngine.hpp"
#include "gridx/matching/orderbook/OrderBookUpdater.hpp"
#include "gridx/matching/validation/OrderValidator.hpp"

namespace gridx::matching::engine {

class OrderProcessor {
public:
    OrderProcessor(adapters::kafka::OrderEventMapper& orderEventMapper,
                   validation::OrderValidator& validator,
                   matching::MatchingEngine& matchingEngine,
                   orderbook::OrderBookUpdater& orderBookUpdater,
                   adapters::kafka::MatchingResultPublisher& matchingResultPublisher);

    void process(std::span<const std::byte> payload);

private:
    adapters::kafka::OrderEventMapper& m_orderEventMapper;
    validation::OrderValidator& m_validator;
    matching::MatchingEngine& m_matchingEngine;
    orderbook::OrderBookUpdater& m_orderBookUpdater;
    adapters::kafka::MatchingResultPublisher& m_matchingResultPublisher;
};

}  // namespace gridx::matching::engine