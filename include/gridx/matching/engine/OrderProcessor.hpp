#pragma once

#include "gridx/matching/adapters/kafka/OrderEventMapper.hpp"
#include "gridx/matching/validation/OrderValidator.hpp"

namespace gridx::matching::engine {

class OrderProcessor {
public:
    OrderProcessor(adapters::kafka::OrderEventMapper mapper, validation::OrderValidator& validator);

    void process(const std::span<const std::byte> payload);

private:
    adapters::kafka::OrderEventMapper mapper_;
    validation::OrderValidator& validator_;
};

}  // namespace gridx::matching::engine