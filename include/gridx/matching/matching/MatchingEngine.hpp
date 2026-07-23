#pragma once

#include <vector>

#include "gridx/matching/domain/Order.hpp"
#include "gridx/matching/domain/Trade.hpp"

namespace gridx::matching::matching {

class SameZoneMatcher;

class MatchingEngine {
public:
    explicit MatchingEngine(SameZoneMatcher& sameZoneMatcher);

    MatchingEngine(const MatchingEngine&) = delete;
    MatchingEngine& operator=(const MatchingEngine&) = delete;
    MatchingEngine(MatchingEngine&&) = delete;
    MatchingEngine& operator=(MatchingEngine&&) = delete;

    ~MatchingEngine() = default;

    [[nodiscard]]
    std::vector<Trade> match(Order incomingOrder);

private:
    SameZoneMatcher& m_sameZoneMatcher;
};

}  // namespace gridx::matching::matching