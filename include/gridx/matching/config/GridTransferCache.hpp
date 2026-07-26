#pragma once

#include "gridx/matching/common/Types.hpp"
#include "gridx/matching/domain/GridTransferRule.hpp"

#include <atomic>
#include <cstddef>
#include <optional>
#include <shared_mutex>
#include <unordered_map>

namespace gridx::matching::config {

struct GridTransferKey {
    GridZoneId sellerGridZone{};
    GridZoneId buyerGridZone{};

    // Automatically generates the equality operator. 
    // The compiler will check if sellerGridZone matches,
    // and then if buyerGridZone matches.
    [[nodiscard]]
    bool operator==(const GridTransferKey&) const noexcept = default;
};

// NOTE: This is a functor (struct acting like a function)
// Turn Grid Transfer Key to a hash number
struct GridTransferKeyHash {
    [[nodiscard]]
    std::size_t operator()(const GridTransferKey& key) const noexcept;  
};

class GridTransferCache {
public:
    GridTransferCache() = default;

    GridTransferCache(const GridTransferCache&) = delete;
    GridTransferCache& operator=(const GridTransferCache&) = delete;

    // Insert rule
    bool upsert(GridTransferRule rule);

    // Remove rule
    bool remove(const GridZoneId sellerGridZone, const GridZoneId buyerGridZone);

    // For lookup of grid transfer rules between 2 zones
    [[nodiscard]]
    std::optional<GridTransferRule> find(
        const GridZoneId sellerGridZone,
        const GridZoneId buyerGridZone
    ) const;

    // For matcher to verify if transfer rules allow
    [[nodiscard]]
    GridTransferRule resolve(
        const GridZoneId sellerGridZone,
        const GridZoneId buyerGridZone
    ) const;

    [[nodiscard]]
    std::size_t size() const;

    // NOTE: noexcept is for optimization that assures
    // the code will never throw an error
    [[nodiscard]]
    bool isReady() const noexcept;

    void markReady() noexcept;

private:
    using RuleMap = std::unordered_map<
        GridTransferKey,
        GridTransferRule,
        GridTransferKeyHash
    >;

    mutable std::shared_mutex mutex_;
    RuleMap rules_;

    std::atomic_bool ready_{false};
};

}