#pragma once

#include <atomic>

#include "gridx/matching/common/Types.hpp"

namespace gridx::matching::matching {

class ITradeIdGenerator {
public:
    virtual ~ITradeIdGenerator() = default;

    [[nodiscard]]
    virtual TradeId next() const = 0;
};

class AtomicTradeIdGenerator final : public ITradeIdGenerator {
public:
    explicit AtomicTradeIdGenerator(TradeId startingId = 1);

    [[nodiscard]]
    TradeId next() const override;

private:
    mutable std::atomic<TradeId> m_nextId;
};

}  // namespace gridx::matching::matching