#pragma once


#ifndef TRADE_HPP
#define TRADE_HPP

#include <cstdint>
#include <string>
#include <chrono>

class Trade {
public:
    uint64_t tradeId;

    std::string symbol;
    
    uint64_t sellId;
    uint64_t buyId;

    uint32_t quantity;
    
    double price;
    time_t timestamp;
    
};

#endif