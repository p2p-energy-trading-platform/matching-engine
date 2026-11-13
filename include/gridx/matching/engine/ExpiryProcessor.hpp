#pragma once

#include <cstddef>

#include "gridx/matching/adapters/ports/IMatchingResultPublisher.hpp"
#include "gridx/matching/expiry/ExpiryManager.hpp"
#include "gridx/matching/orderbook/MarketBookManager.hpp"

namespace gridx::matching::engine {

class ExpiryProcessor {
public:
    ExpiryProcessor(orderbook::MarketBookManager& marketBookManager,
                    const expiry::ExpiryManager& expiryManager,
                    adapters::ports::IMatchingResultPublisher& publisher);

    std::size_t process(Timestamp currentTime);

private:
    orderbook::MarketBookManager& m_marketBookManager;
    const expiry::ExpiryManager& m_expiryManager;
    adapters::ports::IMatchingResultPublisher& m_publisher;
};

}  // namespace gridx::matching::engine
