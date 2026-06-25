#include "order.hpp"

uint32_t Order::getQuantity(){
    return quantity;
}

std::string Order::getSide(){
    return side;
}

std::string Order::getSymbol(){
    return symbol;
}

uint64_t Order::getId(){
    return id;
}

std::string Order::getType(){
    return type;
}

Price Order::getPrice(){
    return price;
}

uint32_t Order::getRemaining(){
    return remaining;
}

Order::Order(uint64_t Id,std::string Side,uint32_t size,std::string company,std::string Type,Price money){
    id = Id;
    side = Side;
    quantity = remaining = size;
    symbol = company;
    type = Type;
    price = money;
}
