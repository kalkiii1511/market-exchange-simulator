#pragma once
#ifndef EXCHANGE_HPP
#define EXCHANGE_HPP

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "matchingEngine.hpp"
#include "orderBook.hpp"
#include "riskEngine.hpp"

struct ExecutionResult {
    bool accepted;
    std::string rejectionReason;
    std::vector<Trade> trades;
};

class Exchange {
public:
    explicit Exchange(RiskLimits riskLimits = {});

    ExecutionResult submitOrder(std::unique_ptr<Order> order);

    bool cancelOrder(OrderId orderId);

    bool modifyOrder(OrderId orderId, Price newPrice, Quantity newQuantity);

    ExecutionResult replaceOrder(OrderId orderId, Price newPrice, Quantity newQuantity);

    bool containsOrder(OrderId orderId) const;

    OrderBook &getOrderBook(const std::string &symbol);

    const OrderBook &getOrderBook(const std::string &symbol) const;

    bool containsSymbol(const std::string &symbol) const;

    RiskEngine &getRiskEngine();

    const RiskEngine &getRiskEngine() const;

    void clear();

private:
    std::unordered_map<std::string, OrderBook> books;
    std::unordered_map<OrderId, std::string> orderSymbols;
    std::unordered_set<OrderId> seenOrderIds;
    MatchingEngine matchingEngine;
    RiskEngine riskEngine;
};

#endif
