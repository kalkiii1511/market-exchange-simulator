#pragma once
#ifndef ORDER_HPP
#define ORDER_HPP
#include <cstdint>
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
    const std::string &getSide() const;
    const std::string &getSymbol() const;
    uint32_t getQuantity() const;
    uint64_t getId() const;
    const std::string &getType() const;
    Price getPrice() const;
    uint32_t getRemaining() const;
    void fill(uint32_t filledQuantity);
    bool isFilled() const;
};


#endif
