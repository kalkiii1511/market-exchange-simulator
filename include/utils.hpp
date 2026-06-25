#pragma once

#ifndef UTILS_HPP
#define UTILS_HPP

#include <cstdint>
#include <list>
#include <memory>
#include <string>

class Order;

struct Price {
    double amount; 
    bool operator<(const Price& other) const {
        return amount<other.amount;
    }
    bool operator>(const Price& other) const {
        return amount>other.amount;
    }
    bool operator==(const Price& other) const {
        return amount==other.amount;
    }
};

struct PriceLevel {
    Price price;
    std::list<std::unique_ptr<Order>> orders;
};

struct OrderLocation {
    std::string side;
    PriceLevel* level;
    std::list<std::unique_ptr<Order>>::iterator orderIterator;
};


#endif
