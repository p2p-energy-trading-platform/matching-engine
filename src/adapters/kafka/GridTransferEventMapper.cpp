#include "gridx/matching/adapters/kafka/GridTransferEventMapper.hpp"
#include <optional>

namespace gridx::matching::adapters::kafka {

std::optional<GridTransferRule> GridTransferEventMapper::toDomain(
    const gridx::grid::v1::GridTransferRule& source) {
    if (source.seller_grid_zone_id() == 0 || source.buyer_grid_zone_id() == 0) {
        return std::nullopt;
    }

    if (source.grid_fee_per_kwh() < 0) {
        return std::nullopt;
    }

    if (source.version() == 0) {
        return std::nullopt;
    }

    if (!source.has_updated_at()) {
        return std::nullopt;
    }

    // NOTE: Possible duplicate code in the other consumer
    // REFACTOR LATER
    const auto timestampSeconds = source.updated_at().seconds();

    const auto timestampNanos = source.updated_at().nanos();

    // Protobuf Timestamp nanos must be in this range.
    if (timestampNanos < 0 || timestampNanos >= 1'000'000'000) {
        return std::nullopt;
    }

    const auto durationSinceEpoch =
        std::chrono::seconds{timestampSeconds} + std::chrono::nanoseconds{timestampNanos};

    const Timestamp updatedAt{std::chrono::duration_cast<Timestamp::duration>(durationSinceEpoch)};

    return GridTransferRule{
        .sellerGridZone = source.seller_grid_zone_id(),
        .buyerGridZone = source.buyer_grid_zone_id(),
        .allowed = source.allowed(),
        .gridFeePerKwh = source.grid_fee_per_kwh(),
        .version = source.version(),
        .updatedAt = updatedAt,
    };
}
}  // namespace gridx::matching::adapters::kafka