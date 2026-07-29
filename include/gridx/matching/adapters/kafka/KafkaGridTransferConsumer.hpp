#pragma once

// #include "gridx/matching/adapters/kafka/GridTransferConsumerConfig.hpp"
#include "gridx/matching/adapters/kafka/KafkaConsumerConfig.hpp"
#include "gridx/matching/config/GridTransferCache.hpp"
#include "gridx/matching/adapters/ports/IKafkaConsumer.hpp"

#include <atomic>
#include <memory>
#include <thread>

namespace RdKafka {
class KafkaConsumer;
class Message;
}  // namespace RdKafka

namespace gridx::matching::adapters::kafka {

class KafkaGridTransferConsumer final : public ports::IKafkaConsumer {
public:
    KafkaGridTransferConsumer(KafkaConsumerConfig config, config::GridTransferCache& cache);

    ~KafkaGridTransferConsumer();

    KafkaGridTransferConsumer(const KafkaGridTransferConsumer&) = delete;

    KafkaGridTransferConsumer& operator=(const KafkaGridTransferConsumer&) = delete;

    // Performs startup bootstrap synchronously.
    // Returns only after the initial cache snapshot is complete.
    void start();

    // Continues consuming future rule changes.
    void startRuntimeUpdates();

    void stop() noexcept;

private:
    void initializeConsumer();
    void runtimeLoop();
    void processMessage(const RdKafka::Message& message);

    KafkaConsumerConfig config_;
    config::GridTransferCache& cache_;

    std::unique_ptr<RdKafka::KafkaConsumer> consumer_;

    std::atomic_bool running_{false};
    std::thread worker_;
};

}  // namespace gridx::matching::adapters::kafka