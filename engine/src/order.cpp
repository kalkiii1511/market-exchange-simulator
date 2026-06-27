#include "order.hpp"
#include <stdexcept>

uint32_t Order::getQuantity() const{
    return quantity;
}

const std::string &Order::getSide() const{
    return side;
}

const std::string &Order::getSymbol() const{
    return symbol;
}

uint64_t Order::getId() const{
    return id;
}

const std::string &Order::getType() const{
    return type;
}

Price Order::getPrice() const{
    return price;
}

uint32_t Order::getRemaining() const{
    return remaining;
}

void Order::fill(uint32_t filledQuantity){
    if (filledQuantity == 0 || filledQuantity > remaining)
        throw std::invalid_argument("Invalid fill quantity.");

    remaining -= filledQuantity;
}

bool Order::isFilled() const{
    return remaining == 0;
}

Order::Order(uint64_t Id,std::string Side,uint32_t size,std::string company,std::string Type,Price money){
    id = Id;
    side = Side;
    quantity = remaining = size;
    symbol = company;
    type = Type;
    price = money;
}
