#include "gridx/matching/matching/EffectivePriceCalculator.hpp"
#include <optional>

namespace gridx::matching::matching {
std::optional<Price> EffectivePriceCalculator::calculateEffectiveAsk(
    Price sellerPrice, GridZoneId sellerZone, GridZoneId buyerZone,
    const config::GridTransferCache& cache) {
    if (sellerZone == buyerZone) {
        return sellerPrice;
    }

    auto rule = cache.find(sellerZone, buyerZone);

    if (!rule.has_value() || !rule->allowed) {
        return std::nullopt;
    }

    return calculateEffectiveAsk(sellerPrice, rule->gridFeePerKwh);
}

std::optional<Price> EffectivePriceCalculator::calculateEffectiveBid(
    Price buyerLimitPrice, GridZoneId sellerZone, GridZoneId buyerZone,
    const config::GridTransferCache& cache) {
    if (sellerZone == buyerZone) {
        return buyerLimitPrice;
    }

    auto rule = cache.find(sellerZone, buyerZone);

    if (!rule.has_value() || !rule->allowed) {
        return std::nullopt;
    }

    return calculateEffectiveBid(buyerLimitPrice, rule->gridFeePerKwh);
}

}  // namespace gridx::matching::matching