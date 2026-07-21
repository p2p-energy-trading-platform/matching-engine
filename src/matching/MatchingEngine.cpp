#include "gridx/matching/matching/MatchingEngine.hpp"
#include "gridx/matching/matching/SameZoneMatcher.hpp"

#include <utility>

namespace gridx::matching::matching
{

MatchingEngine::MatchingEngine(SameZoneMatcher& sameZoneMatcher)
    : m_sameZoneMatcher(sameZoneMatcher)
{
    // Dependencies are injected during construction.
}

std::vector<Trade> MatchingEngine::match(Order incomingOrder)
{
    // Delegate matching to the same-zone matcher.
    return m_sameZoneMatcher.match(std::move(incomingOrder));
}

} // namespace gridx::matching::matching