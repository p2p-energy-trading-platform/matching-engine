#pragma once

#include "gridx/matching/domain/MatchingResult.hpp"

namespace gridx::matching::adapters::kafka {

class KafkaProducer;

class MatchingEventPublisher {
public:
    explicit MatchingEventPublisher(KafkaProducer& producer);

    MatchingEventPublisher(const MatchingEventPublisher&) = delete;
    MatchingEventPublisher& operator=(const MatchingEventPublisher&) = delete;

    MatchingEventPublisher(MatchingEventPublisher&&) noexcept = delete;
    MatchingEventPublisher& operator=(MatchingEventPublisher&&) noexcept = delete;

    /**
     * Publishes all events generated during a completed matching cycle.
     *
     * Trade events are published before order status events.
     * This method must be called only after the order book has been updated.
     */
    void publish(const MatchingResult& result);

private:
    /**
     * Publishes all executed trade events.
     */
    void publishTrades(const MatchingResult& result);

    /**
     * Publishes order lifecycle events.
     */
    void publishOrderUpdates(const MatchingResult& result);

private:
    KafkaProducer& m_producer;
};

}  // namespace gridx::matching::adapters::kafka