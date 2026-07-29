#pragma once

namespace gridx::matching::adapters::ports {

class IKafkaConsumer {
public:
    virtual ~IKafkaConsumer() = default;

    virtual void start() = 0;

    virtual void stop() noexcept = 0;
};

}  // namespace gridx::matching::adapters::ports