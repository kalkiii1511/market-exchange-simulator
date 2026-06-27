#include "riskEngine.hpp"

#include <cmath>
#include <utility>

RiskEngine::RiskEngine(RiskLimits limits) : limits(std::move(limits)) {}

RiskResult RiskEngine::validate(const Order &order, bool duplicateOrderId) const
{
    if (duplicateOrderId)
        return {false, "Duplicate order ID."};
    if (order.getId() == 0)
        return {false, "Order ID must be non-zero."};
    if (order.getSymbol().empty())
        return {false, "Symbol must not be empty."};
    if (order.getSide() != "BUY" && order.getSide() != "SELL")
        return {false, "Side must be BUY or SELL."};
    if (order.getType() != "LIMIT")
        return {false, "Only LIMIT orders are supported."};
    if (order.getQuantity() == 0)
        return {false, "Quantity must be positive."};
    if (order.getQuantity() > limits.maxOrderQuantity)
        return {false, "Order quantity exceeds the risk limit."};
    if (!std::isfinite(order.getPrice().amount) || order.getPrice().amount <= 0.0)
        return {false, "Price must be finite and positive."};

    const double notional = order.getPrice().amount * order.getQuantity();
    if (!std::isfinite(notional) || notional > limits.maxOrderNotional)
        return {false, "Order notional exceeds the risk limit."};

    return {true, {}};
}

const RiskLimits &RiskEngine::getLimits() const
{
    return limits;
}

void RiskEngine::setLimits(const RiskLimits &newLimits)
{
    limits = newLimits;
}
