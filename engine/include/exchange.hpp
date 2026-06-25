#pragma once
#ifndef EXCHANGE_HPP
#define EXCHANGE_HPP

#include <memory>
#include <string>
#include <unordered_map>
#include "orderBook.hpp"

class Exchange {
public:
    void submitOrder(std::unique_ptr<Order> order);

    bool cancelOrder(OrderId orderId);

    bool modifyOrder(OrderId orderId, Price newPrice, Quantity newQuantity);

    bool containsOrder(OrderId orderId) const;

    OrderBook &getOrderBook(const std::string &symbol);

    const OrderBook &getOrderBook(const std::string &symbol) const;

    bool containsSymbol(const std::string &symbol) const;

    void clear();

private:
    std::unordered_map<std::string, OrderBook> books;
    std::unordered_map<OrderId, std::string> orderSymbols;
};

#endif
