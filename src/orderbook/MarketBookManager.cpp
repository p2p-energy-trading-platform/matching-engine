#include "gridx/matching/orderbook/MarketBookManager.hpp"

namespace gridx::matching::orderbook {

MarketBook& MarketBookManager::getOrCreate(const MarketId& marketId) {
    auto [iterator, inserted] = marketBooks_.try_emplace(marketId, marketId);

    return iterator->second;
}

MarketBook* MarketBookManager::find(const MarketId& marketId) {
    auto iterator = marketBooks_.find(marketId);

    if (iterator == marketBooks_.end()) {
        return nullptr;
    }

    return &iterator->second;
}

const MarketBook* MarketBookManager::find(const MarketId& marketId) const {
    auto iterator = marketBooks_.find(marketId);

    if (iterator == marketBooks_.end()) {
        return nullptr;
    }

    return &iterator->second;
}

void MarketBookManager::clear() noexcept {
    marketBooks_.clear();
}

std::size_t MarketBookManager::size() const noexcept {
    return marketBooks_.size();
}

bool MarketBookManager::empty() const noexcept {
    return marketBooks_.empty();
}

}  // namespace gridx::matching::orderbook