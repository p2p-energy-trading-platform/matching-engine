#include <optional>
#include "gridx/matching/common/Types.hpp"
#include "gridx/matching/config/GridTransferCache.hpp"

namespace gridx::matching::matching {

class EffectivePriceCalculator {
public:
    [[nodiscard]]
    static constexpr Price calculateEffectiveAsk(Price sellerPrice, GridFee gridFee) noexcept
    {
        return sellerPrice + gridFee;
    }

    [[nodiscard]]
    static std::optional<Price> calculateEffectiveAsk(Price sellerPrice, GridZoneId sellerZone, GridZoneId buyerZone, const config::GridTransferCache& cache);

    [[nodiscard]]
    static constexpr Price calculateEffectiveBid(Price buyerLimitPrice, GridFee gridFee) noexcept
    {
        return buyerLimitPrice - gridFee;
    }

    [[nodiscard]]
    static std::optional<Price> calculateEffectiveBid(Price buyerLimitPrice, GridZoneId sellerZone, GridZoneId buyerZone, const config::GridTransferCache& cache);
};

} // namespace gridx::matching::matching