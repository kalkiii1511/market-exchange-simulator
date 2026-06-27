#pragma once
#ifndef RISK_ENGINE_HPP
#define RISK_ENGINE_HPP

#include <cstdint>
#include <string>
#include "order.hpp"

struct RiskLimits {
    uint32_t maxOrderQuantity = 1'000'000;
    double maxOrderNotional = 1'000'000'000.0;
};

struct RiskResult {
    bool accepted;
    std::string reason;
};

class RiskEngine {
public:
    explicit RiskEngine(RiskLimits limits = {});

    RiskResult validate(const Order &order, bool duplicateOrderId) const;
    const RiskLimits &getLimits() const;
    void setLimits(const RiskLimits &newLimits);

private:
    RiskLimits limits;
};

#endif
