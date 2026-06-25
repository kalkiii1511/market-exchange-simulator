#pragma once
#ifndef SIDEBOOK_HPP
#define SIDEBOOK_HPP

#include <memory>
#include <map>
#include "utils.hpp"
template <typename Comparator>
class SideBook
{
private:
    std::map<Price, PriceLevel, Comparator> levels;

public:
    void addOrder(std::unique_ptr<Order> order);

    void cancelOrder(const OrderLocation &location);

    PriceLevel &bestPriceLevel();

    bool empty() const;

    bool containsPrice(const Price &price) const;

    PriceLevel &getPriceLevel(const Price &price);

    void removePriceLevel(const Price &price);
};
#endif