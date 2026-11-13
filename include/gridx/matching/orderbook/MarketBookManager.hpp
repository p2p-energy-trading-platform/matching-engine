#pragma once

#include <map>

#include "gridx/matching/domain/MarketId.hpp"
#include "gridx/matching/orderbook/MarketBook.hpp"

namespace gridx::matching::orderbook {

/**
 * Owns and manages all Market Books in memory.
 *
 * Each Market Book represents a single delivery slot.
 * Orders with different delivery slots are stored in different
 * Market Books and are never matched against each other.
 */
class MarketBookManager {
public:
    MarketBookManager() = default;
    ~MarketBookManager() = default;

    MarketBookManager(const MarketBookManager&) = delete;
    MarketBookManager& operator=(const MarketBookManager&) = delete;

    MarketBookManager(MarketBookManager&&) = delete;
    MarketBookManager& operator=(MarketBookManager&&) = delete;

    /**
     * Returns the Market Book for the specified market.
     *
     * If the Market Book does not already exist, a new one is created.
     */
    MarketBook& getOrCreate(const MarketId& marketId);

    /**
     * Finds an existing Market Book.
     *
     * @return Pointer to the Market Book, or nullptr if it does not exist.
     */
    [[nodiscard]]
    MarketBook* find(const MarketId& marketId);

    /**
     * Finds an existing Market Book.
     *
     * @return Pointer to the Market Book, or nullptr if it does not exist.
     */
    [[nodiscard]]
    const MarketBook* find(const MarketId& marketId) const;

    /**
     * Calls fn(MarketBook&) for every Market Book, in delivery slot order.
     *
     * fn must not add or remove Market Books.
     */
    template <typename Fn>
    void forEach(Fn&& fn) {
        for (auto& [marketId, marketBook] : marketBooks_) {
            (void) marketId;
            fn(marketBook);
        }
    }

    /**
     * Removes the Market Book for the specified market.
     *
     * @return true if a Market Book was removed.
     */
    bool erase(const MarketId& marketId);

    /**
     * Removes all Market Books.
     *
     * Primarily intended for testing or engine reset.
     */
    void clear() noexcept;

    /**
     * Returns the number of active Market Books.
     */
    [[nodiscard]]
    std::size_t size() const noexcept;

    /**
     * Returns true if no Market Books are present.
     */
    [[nodiscard]]
    bool empty() const noexcept;

private:
    std::map<MarketId, MarketBook> marketBooks_;
};

}  // namespace gridx::matching::orderbook