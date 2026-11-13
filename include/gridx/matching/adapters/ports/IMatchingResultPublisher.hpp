#pragma once

#include "gridx/matching/domain/MatchingResult.hpp"

namespace gridx::matching::adapters::ports {

// Interface for publishing matching results.
class IMatchingResultPublisher {
public:
    virtual ~IMatchingResultPublisher() = default;

    virtual void publish(const MatchingResult& result) = 0;
};

}  // namespace gridx::matching::adapters::ports
