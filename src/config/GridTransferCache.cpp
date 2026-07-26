#include "gridx/matching/config//GridTransferCache.hpp"
#include <atomic>
#include <cstddef>
#include <functional>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include "gridx/matching/domain/GridTransferRule.hpp"

namespace gridx::matching::config {

std::size_t GridTransferKeyHash::operator()(const GridTransferKey& key) const noexcept
{
    const auto sellerHash = 
        std::hash<GridZoneId>{}(key.sellerGridZone);
    const auto buyerHash = 
        std::hash<GridZoneId>{}(key.buyerGridZone);

    // Weird code but explanation is below
    // We cannot add 2 hashes together. 
    // Why? Due to hash collisions and adding does not prevent collisions.
    // The below uses a hash combination algorithm
    // 0x9e3779b9U is a constant from golden ratio that acts as a
    // mathematical sequence to ensure bits are scattered evenly
    // The shift operation scrambles the bits of first hash. Why?
    // We are trying to avoid a scenario where A to B and B to A results
    // in same hash. With the bit shift on first hash we can gurantee that
    // A to B and B to A won't result in a hash collision
    return sellerHash ^
           (buyerHash + 0x9e3779b9U +
            (sellerHash << 6U) +
            (sellerHash >> 2U));
}

bool GridTransferCache::upsert(GridTransferRule rule)
{
    if (rule.sellerGridZone == rule.buyerGridZone)
    {
        return false;
    }

    if (rule.gridFeePerKwh < 0)
    {
        return false;
    }

    const GridTransferKey key{
        .sellerGridZone = rule.sellerGridZone,
        .buyerGridZone = rule.buyerGridZone
    };

    std::unique_lock lock{mutex_};

    const auto existing = rules_.find(key);

    if (existing != rules_.end() && rule.version <= existing->second.version)
    {
        return false;
    }

    rules_.insert_or_assign(key, std::move(rule));
    return true;
}

bool GridTransferCache::remove(
    const GridZoneId sellerGridZone,
    const GridZoneId buyerGridZone)
{
    if (sellerGridZone == buyerGridZone)
    {
        return false;
    }

    std::unique_lock lock{mutex_};

    const auto removed =  rules_.erase(GridTransferKey{
        .sellerGridZone = sellerGridZone,
        .buyerGridZone = buyerGridZone
    });

    // If no of items removed is greater than 0,
    // return true else false
    return removed > 0;
}

std::optional<GridTransferRule> GridTransferCache::find(
    const GridZoneId sellerGridZone,
    const GridZoneId buyerGridZone
) const
{
    if (sellerGridZone == buyerGridZone)
    {
        return GridTransferRule{
            .sellerGridZone = sellerGridZone,
            .buyerGridZone = buyerGridZone,
            .allowed = true,
            .gridFeePerKwh = 0,
            .version = 0,
            .updatedAt = {}
        };
    }

    std::shared_lock lock{mutex_};

    const auto iterator = rules_.find(GridTransferKey{
        .sellerGridZone = sellerGridZone,
        .buyerGridZone = buyerGridZone
    });

    if (iterator == rules_.end())
    {
        return std::nullopt;
    }

    return iterator->second;
}

GridTransferRule GridTransferCache::resolve(
    const GridZoneId sellerGridZone,
    const GridZoneId buyerGridZone
) const
{
    if (sellerGridZone == buyerGridZone)
    {
        return GridTransferRule{
            .sellerGridZone = sellerGridZone,
            .buyerGridZone = buyerGridZone,
            .allowed = true,
            .gridFeePerKwh = 0,
            .version = 0,
            .updatedAt = {}
        };
    }

    const auto rule = find(sellerGridZone, buyerGridZone);

    if (rule.has_value())
    {
        return *rule;
    }

    return GridTransferRule{
        .sellerGridZone = sellerGridZone,
        .buyerGridZone = buyerGridZone,
        .allowed = false,
        .gridFeePerKwh = 0,
        .version = 0,
        .updatedAt = {}
    };
}

std::size_t GridTransferCache::size() const
{
    // create a shared reader lock
    std::shared_lock lock{mutex_};
    return rules_.size();
}

bool GridTransferCache::isReady() const noexcept
{
    // Directly loads the underlying value of the atomic_bool
    // Bypasses OS's thread scheduler
    // memory_order_acquire => low-level memory fence 
    // instruction given to the CPU and compiler
    return ready_.load(std::memory_order_acquire);
}

void GridTransferCache::markReady() noexcept
{
    // Lock free status publishing
    // Atomically overwrites to true
    ready_.store(true, std::memory_order_release);
}

}