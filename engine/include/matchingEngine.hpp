#pragma once
#ifndef MATCHING_ENGINE_HPP
#define MATCHING_ENGINE_HPP

#include <cstdint>
#include <memory>
#include <vector>
#include "orderBook.hpp"
#include "trade.hpp"

class MatchingEngine {
public:
    std::vector<Trade> execute(OrderBook &book, std::unique_ptr<Order> incomingOrder);

private:
    uint64_t nextTradeId = 1;

    Trade createTrade(const Order &buyOrder, const Order &sellOrder,
                      uint32_t quantity, Price price);
};

#endif
