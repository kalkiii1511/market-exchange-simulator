#pragma once
#ifndef ORDER_HPP
#define ORDER_HPP
#include <string>
#include "utils.hpp"


class Order {
private:
    uint64_t id;
    std::string side;
    uint32_t quantity;
    std::string symbol;
    std::string type;
    Price price;
    uint32_t remaining;

public:

    Order(uint64_t Id,std::string Side,uint32_t size,std::string company,std::string Type,Price money);
    std::string getSide();
    std::string getSymbol();
    uint32_t getQuantity();
    uint64_t getId();
    std::string getType();
    Price getPrice();
    uint32_t getRemaining();
};


#endif