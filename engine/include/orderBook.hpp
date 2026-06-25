#pragma once
#ifndef ORDERBOOK_HPP
#define ORDERBOOK_HPP

#include <functional>
#include <memory>
#include <unordered_map>
#include "sideBook.hpp"

using OrderId = uint64_t;
using Quantity = uint32_t;
using BidBook = SideBook<std::greater<Price>>;
using AskBook = SideBook<std::less<Price>>;

class OrderBook {
public:
    void addOrder(std::unique_ptr<Order> order);

    bool cancelOrder(OrderId orderId);

    bool modifyOrder(OrderId orderId,Price newPrice,Quantity newQuantity);

    PriceLevel &bestBid();

    PriceLevel &bestAsk();

    bool containsOrder(OrderId orderId) const;

    OrderLocation &getOrderLocation(OrderId orderId);

    void removeOrderIndex(OrderId orderId);

    void addOrderIndex(OrderId orderId,const OrderLocation &location);

    bool empty() const;

    void clear();

private:
    BidBook bids;

    AskBook asks;

    std::unordered_map<OrderId, OrderLocation> orderIndex;
};

#endif
