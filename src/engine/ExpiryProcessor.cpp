#include "gridx/matching/engine/ExpiryProcessor.hpp"

#include <utility>

#include <spdlog/spdlog.h>

namespace gridx::matching::engine {

ExpiryProcessor::ExpiryProcessor(orderbook::MarketBookManager& marketBookManager,
                                 const expiry::ExpiryManager& expiryManager,
                                 adapters::ports::IMatchingResultPublisher& publisher)
    : m_marketBookManager(marketBookManager)
    , m_expiryManager(expiryManager)
    , m_publisher(publisher) {}

std::size_t ExpiryProcessor::process(Timestamp currentTime) {
    auto expiredUpdates = m_expiryManager.expireMarkets(m_marketBookManager, currentTime);

    if (expiredUpdates.empty()) {
        return 0;
    }

    const auto expiredCount = expiredUpdates.size();

    MatchingResult result{};
    result.orderUpdates = std::move(expiredUpdates);

    m_publisher.publish(result);

    spdlog::info("Expired {} order(s)", expiredCount);

    return expiredCount;
}

}  // namespace gridx::matching::engine
