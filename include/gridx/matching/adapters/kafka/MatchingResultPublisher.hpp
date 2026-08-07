#pragma once

#include "gridx/matching/adapters/kafka/KafkaProducer.hpp"
#include "gridx/matching/adapters/kafka/OrderUpdateEventMapper.hpp"
#include "gridx/matching/adapters/kafka/TradeEventMapper.hpp"
#include "gridx/matching/domain/MatchingResult.hpp"

namespace gridx::matching::adapters::kafka {

class MatchingResultPublisher {
public:
    MatchingResultPublisher(KafkaProducer& producer, TradeEventMapper& tradeEventMapper,
                            OrderUpdateEventMapper& orderUpdateEventMapper);

    /**
     * Publishes all events produced during a matching cycle.
     */
    void publish(const MatchingResult& result);

private:
    void publishTrades(const MatchingResult& result);

    void publishOrderUpdates(const MatchingResult& result);

    KafkaProducer& m_producer;
    TradeEventMapper& m_tradeEventMapper;
    OrderUpdateEventMapper& m_orderUpdateEventMapper;
};

}  // namespace gridx::matching::adapters::kafka