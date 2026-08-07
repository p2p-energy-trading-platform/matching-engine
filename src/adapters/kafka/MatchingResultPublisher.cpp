#include "gridx/matching/adapters/kafka/MatchingResultPublisher.hpp"

#include "gridx/matching/adapters/kafka/KafkaTopics.hpp"
#include "gridx/matching/adapters/kafka/ProtobufCodec.hpp"

namespace gridx::matching::adapters::kafka {

MatchingResultPublisher::MatchingResultPublisher(KafkaProducer& producer,
                                                 TradeEventMapper& tradeEventMapper,
                                                 OrderUpdateEventMapper& orderUpdateEventMapper)
    : m_producer(producer)
    , m_tradeEventMapper(tradeEventMapper)
    , m_orderUpdateEventMapper(orderUpdateEventMapper) {}

void MatchingResultPublisher::publish(const MatchingResult& result) {
    publishTrades(result);
    publishOrderUpdates(result);
}

void MatchingResultPublisher::publishTrades(const MatchingResult& result) {
    for (const auto& trade : result.trades) {
        const auto event = m_tradeEventMapper.toProtobuf(trade);

        const auto payload = ProtobufCodec::serialize(event);

        m_producer.send(topics::kTradeExecuted, payload);
    }
}

void MatchingResultPublisher::publishOrderUpdates(const MatchingResult& result) {
    for (const auto& update : result.orderUpdates) {
        const auto event = m_orderUpdateEventMapper.toProtobuf(update);

        const auto payload = ProtobufCodec::serialize(event);

        m_producer.send(topics::kOrderUpdated, payload);
    }
}

}  // namespace gridx::matching::adapters::kafka