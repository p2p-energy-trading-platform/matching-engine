#pragma once

#include <chrono>
#include <memory>

#include "gridx/matching/domain/Order.hpp"
#include "gridx/matching/domain/Trade.hpp"
#include "gridx/matching/matching/TradeIdGenerator.hpp"

namespace gridx::matching::test_support {

inline Timestamp fixedTimestamp() {
    return Timestamp{} + std::chrono::hours{24 * 365 * 100};
}

template <typename Rep, typename Period>
inline Timestamp after(const Timestamp& base, const std::chrono::duration<Rep, Period>& delta) {
    return base + std::chrono::duration_cast<Timestamp::duration>(delta);
}

inline MarketId makeMarketId(const Timestamp& deliverySlotStart = fixedTimestamp()) {
    return MarketId{.deliverySlotStart = deliverySlotStart};
}

template <typename Id>
class SequentialIdGenerator {
public:
    explicit SequentialIdGenerator(Id start = Id{1}) : m_nextId(start) {}

    [[nodiscard]] Id next() {
        return m_nextId++;
    }

    [[nodiscard]] Id current() const {
        return m_nextId;
    }

private:
    Id m_nextId;
};

using OrderIdGenerator = SequentialIdGenerator<OrderId>;
using TradeIdGenerator = SequentialIdGenerator<TradeId>;
using UserIdGenerator = SequentialIdGenerator<UserId>;
using GridZoneIdGenerator = SequentialIdGenerator<GridZoneId>;

class TradeIdGeneratorAdapter final : public matching::ITradeIdGenerator {
public:
    explicit TradeIdGeneratorAdapter(TradeId start = TradeId{1}) : m_generator(start) {}

    [[nodiscard]] TradeId next() const override {
        return m_generator.next();
    }

private:
    mutable TradeIdGenerator m_generator;
};

class OrderBuilder {
public:
    OrderBuilder() {
        m_order.orderId = 1;
        m_order.userId = 1;
        m_order.marketId = makeMarketId();
        m_order.gridZone = 1;
        m_order.side = Side::Buy;
        m_order.orderType = OrderType::Limit;
        m_order.status = OrderStatus::New;
        m_order.price = 100;
        m_order.quantity = 10;
        m_order.remainingQuantity = 10;
        m_order.createdAt = fixedTimestamp();
        m_order.expiresAt = after(m_order.createdAt, std::chrono::hours{1});
    }

    [[nodiscard]] OrderBuilder& withOrderId(const OrderId orderId) {
        m_order.orderId = orderId;
        return *this;
    }

    [[nodiscard]] OrderBuilder& withUserId(const UserId userId) {
        m_order.userId = userId;
        return *this;
    }

    [[nodiscard]] OrderBuilder& withMarketId(const MarketId& marketId) {
        m_order.marketId = marketId;
        return *this;
    }

    [[nodiscard]] OrderBuilder& withDeliverySlotStart(const Timestamp& deliverySlotStart) {
        m_order.marketId.deliverySlotStart = deliverySlotStart;
        return *this;
    }

    [[nodiscard]] OrderBuilder& withGridZone(const GridZoneId gridZone) {
        m_order.gridZone = gridZone;
        return *this;
    }

    [[nodiscard]] OrderBuilder& withSide(const Side side) {
        m_order.side = side;
        return *this;
    }

    [[nodiscard]] OrderBuilder& buy() {
        return withSide(Side::Buy);
    }

    [[nodiscard]] OrderBuilder& sell() {
        return withSide(Side::Sell);
    }

    [[nodiscard]] OrderBuilder& withOrderType(const OrderType orderType) {
        m_order.orderType = orderType;
        return *this;
    }

    [[nodiscard]] OrderBuilder& withStatus(const OrderStatus status) {
        m_order.status = status;
        return *this;
    }

    [[nodiscard]] OrderBuilder& withPrice(const Price price) {
        m_order.price = price;
        return *this;
    }

    [[nodiscard]] OrderBuilder& withQuantity(const Quantity quantity) {
        m_order.quantity = quantity;
        m_order.remainingQuantity = quantity;
        return *this;
    }

    [[nodiscard]] OrderBuilder& withRemainingQuantity(const Quantity remainingQuantity) {
        m_order.remainingQuantity = remainingQuantity;
        return *this;
    }

    [[nodiscard]] OrderBuilder& withCreatedAt(const Timestamp& createdAt) {
        m_order.createdAt = createdAt;
        return *this;
    }

    [[nodiscard]] OrderBuilder& withExpiresAt(const Timestamp& expiresAt) {
        m_order.expiresAt = expiresAt;
        return *this;
    }

    template <typename Rep, typename Period>
    [[nodiscard]] OrderBuilder& withLifetime(const std::chrono::duration<Rep, Period>& lifetime) {
        m_order.expiresAt = after(m_order.createdAt, lifetime);
        return *this;
    }

    [[nodiscard]] Order build() const {
        return m_order;
    }

    [[nodiscard]] std::shared_ptr<Order> buildPtr() const {
        return std::make_shared<Order>(m_order);
    }

private:
    Order m_order{};
};

class TradeBuilder {
public:
    TradeBuilder() {
        m_trade.tradeId = 1;
        m_trade.buyOrderId = 1;
        m_trade.sellOrderId = 2;
        m_trade.buyerId = 1;
        m_trade.sellerId = 2;
        m_trade.buyerGridZone = 1;
        m_trade.sellerGridZone = 2;
        m_trade.deliverySlotStart = fixedTimestamp();
        m_trade.deliverySlotEnd = after(m_trade.deliverySlotStart, std::chrono::minutes{30});
        m_trade.energyPrice = 100;
        m_trade.gridFee = 0;
        m_trade.quantity = 10;
        m_trade.buyerTotalPrice = m_trade.energyPrice * m_trade.quantity;
        m_trade.gridRuleVersion = 1;
        m_trade.timestamp = fixedTimestamp();
    }

    [[nodiscard]] TradeBuilder& withTradeId(const TradeId tradeId) {
        m_trade.tradeId = tradeId;
        return *this;
    }

    [[nodiscard]] TradeBuilder& withBuyOrderId(const OrderId orderId) {
        m_trade.buyOrderId = orderId;
        return *this;
    }

    [[nodiscard]] TradeBuilder& withSellOrderId(const OrderId orderId) {
        m_trade.sellOrderId = orderId;
        return *this;
    }

    [[nodiscard]] TradeBuilder& withBuyerId(const UserId userId) {
        m_trade.buyerId = userId;
        return *this;
    }

    [[nodiscard]] TradeBuilder& withSellerId(const UserId userId) {
        m_trade.sellerId = userId;
        return *this;
    }

    [[nodiscard]] TradeBuilder& withBuyerGridZone(const GridZoneId gridZone) {
        m_trade.buyerGridZone = gridZone;
        return *this;
    }

    [[nodiscard]] TradeBuilder& withSellerGridZone(const GridZoneId gridZone) {
        m_trade.sellerGridZone = gridZone;
        return *this;
    }

    [[nodiscard]] TradeBuilder& withDeliverySlotStart(const Timestamp& deliverySlotStart) {
        m_trade.deliverySlotStart = deliverySlotStart;
        m_trade.deliverySlotEnd = after(deliverySlotStart, std::chrono::minutes{30});
        return *this;
    }

    [[nodiscard]] TradeBuilder& withDeliverySlotEnd(const Timestamp& deliverySlotEnd) {
        m_trade.deliverySlotEnd = deliverySlotEnd;
        return *this;
    }

    [[nodiscard]] TradeBuilder& withEnergyPrice(const Price energyPrice) {
        m_trade.energyPrice = energyPrice;
        return *this;
    }

    [[nodiscard]] TradeBuilder& withGridFee(const GridFee gridFee) {
        m_trade.gridFee = gridFee;
        return *this;
    }

    [[nodiscard]] TradeBuilder& withQuantity(const Quantity quantity) {
        m_trade.quantity = quantity;
        return *this;
    }

    [[nodiscard]] TradeBuilder& withBuyerTotalPrice(const Price buyerTotalPrice) {
        m_trade.buyerTotalPrice = buyerTotalPrice;
        return *this;
    }

    [[nodiscard]] TradeBuilder& withGridRuleVersion(const GridRuleVersion gridRuleVersion) {
        m_trade.gridRuleVersion = gridRuleVersion;
        return *this;
    }

    [[nodiscard]] TradeBuilder& withTimestamp(const Timestamp& timestamp) {
        m_trade.timestamp = timestamp;
        return *this;
    }

    [[nodiscard]] Trade build() const {
        return m_trade;
    }

private:
    Trade m_trade{};
};

}  // namespace gridx::matching::test_support