#pragma once

#include "gridx/matching/adapters/kafka/KafkaConsumerConfig.hpp"

namespace RdKafka {
class KafkaConsumer;
}

// TODO: Initialize kafka consumer methods and add more methods

namespace gridx::matching::adapters::kafka {

class KafkaOrderConsumer {
public:
    ~KafkaOrderConsumer();

    KafkaOrderConsumer(const KafkaConsumerConfig&) = delete;
    KafkaOrderConsumer& operator=(const KafkaOrderConsumer&) = delete;

    void start();
    void stop();
};

}  // namespace gridx::matching::adapters::kafka