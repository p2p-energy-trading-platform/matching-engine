#pragma once

namespace gridx::matching::orderbook {

template <typename Comparator>
RestingOrderIterator<Comparator>::RestingOrderIterator(
    PriceLevelIterator levelIt,
    PriceLevelIterator levelEnd)
    : m_levelIt(levelIt),
      m_levelEnd(levelEnd) {

    if (m_levelIt != m_levelEnd) {
        m_queueIt = m_levelIt->second.cbegin();
    }

    skipEmptyPriceLevels();
}

template <typename Comparator>
void RestingOrderIterator<Comparator>::skipEmptyPriceLevels() {

    while (m_levelIt != m_levelEnd) {

        if (m_queueIt != m_levelIt->second.cend()) {
            return;
        }

        ++m_levelIt;

        if (m_levelIt != m_levelEnd) {
            m_queueIt = m_levelIt->second.cbegin();
        }
    }
}

template <typename Comparator>
const OrderPtr&
RestingOrderIterator<Comparator>::operator*() const noexcept {
    return *m_queueIt;
}

template <typename Comparator>
const OrderPtr*
RestingOrderIterator<Comparator>::operator->() const noexcept {
    return &(*m_queueIt);
}

template <typename Comparator>
RestingOrderIterator<Comparator>&
RestingOrderIterator<Comparator>::operator++() {

    ++m_queueIt;

    skipEmptyPriceLevels();

    return *this;
}

template <typename Comparator>
bool RestingOrderIterator<Comparator>::operator==(
    const RestingOrderIterator& other) const noexcept {

    if (m_levelIt == m_levelEnd &&
        other.m_levelIt == other.m_levelEnd) {
        return true;
    }

    return m_levelIt == other.m_levelIt &&
           m_queueIt == other.m_queueIt;
}

template <typename Comparator>
bool RestingOrderIterator<Comparator>::operator!=(
    const RestingOrderIterator& other) const noexcept {

    return !(*this == other);
}

} // namespace gridx::matching::orderbook