#pragma once
#ifndef TRADE_HPP
#define TRADE_HPP

#include <cstdint>
#include <string>
#include <chrono>
#include "utils.hpp"

struct Trade {

    uint64_t tradeId;

    std::string symbol;

    uint64_t buyId;
    uint64_t sellId;


    uint32_t quantity;
    
    Price price;
    std::chrono::system_clock::time_point timestamp;
    
};

#endif