#pragma once

#include "gridx/matching/adapters/kafka/KafkaConsumerConfig.hpp"
#include "gridx/matching/engine/OrderProcessor.hpp"

#include <atomic>
#include <memory>

namespace RdKafka {
class KafkaConsumer;
}

namespace gridx::matching::adapters::kafka {

class KafkaOrderConsumer {
public:
    KafkaOrderConsumer(KafkaConsumerConfig config, engine::OrderProcessor& orderProcessor);

    ~KafkaOrderConsumer();

    KafkaOrderConsumer(const KafkaOrderConsumer&) = delete;
    KafkaOrderConsumer& operator=(const KafkaOrderConsumer&) = delete;

    KafkaOrderConsumer(KafkaOrderConsumer&&) = delete;
    KafkaOrderConsumer& operator=(KafkaOrderConsumer&&) = delete;

    void start();
    void stop() noexcept;

private:
    void initializeConsumer();
    void consumeLoop();

    KafkaConsumerConfig config_;
    engine::OrderProcessor& orderProcessor_;

    std::unique_ptr<RdKafka::KafkaConsumer> consumer_;
    std::atomic_bool running_{false};
};

}  // namespace gridx::matching::adapters::kafka