#include "gridx/matching/adapters/kafka/GridTransferEventMapper.hpp"
#include <optional>

namespace gridx::matching::adapters::kafka {

std::optional<GridTransferRule> GridTransferEventMapper::toDomain(
    const gridx::grid::v1::GridTransferRule& source
)
{
    if (source.seller_grid_zone_id() == 0 || source.buyer_grid_zone_id() == 0)
    {
        return std::nullopt;
    }

    if (source.grid_fee_per_kwh() < 0)
    {
        return std::nullopt;
    }

    if (source.version() == 0)
    {
        return std::nullopt;
    }

    if (!source.has_updated_at())
    {
        return std::nullopt;
    }

    const auto seconds = std::chrono::seconds{
        source.updated_at().seconds()
    };

    const auto nanos = std::chrono::nanoseconds{
        source.updated_at().nanos()
    };

    return GridTransferRule{
        .sellerGridZone = source.seller_grid_zone_id(),
        .buyerGridZone = source.buyer_grid_zone_id(),
        .allowed = source.allowed(),
        .gridFeePerKwh =
            source.grid_fee_per_kwh(),
        .version = source.version(),
        .updatedAt = Timestamp{seconds + nanos},
    };
}
} // namespace gridx::matching::adapters::kafka