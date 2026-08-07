#include "gridx/matching/matching/TradeIdGenerator.hpp"

namespace gridx::matching::matching {

AtomicTradeIdGenerator::AtomicTradeIdGenerator(TradeId startingId) : m_nextId(startingId) {}

TradeId AtomicTradeIdGenerator::next() const {
    return m_nextId.fetch_add(1, std::memory_order_relaxed);
}

}  // namespace gridx::matching::matching