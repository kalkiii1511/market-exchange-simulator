#include "sideBook.hpp"

template <typename Comparator>
void SideBook<Comparator>::addOrder(std::unique_ptr<Order> order)
{
    Price price = order->getPrice();

    auto &level = levels[price];
    level.price = price;
    level.orders.push_back(std::move(order));
}

template <typename Comparator>
void SideBook<Comparator>::cancelOrder(const OrderLocation &location)
{
    location.level->orders.erase(location.it);
}

template <typename Comparator>
PriceLevel &SideBook<Comparator>::bestPriceLevel()
{
    if (levels.empty())
        throw std::runtime_error("Order book is empty.");

    return levels.begin()->second;
}

template <typename Comparator>
bool SideBook<Comparator>::empty() const
{
    return levels.empty();
}

template <typename Comparator>
bool SideBook<Comparator>::containsPrice(const Price &price) const
{
    return levels.find(price) != levels.end();
}

template <typename Comparator>
PriceLevel &SideBook<Comparator>::getPriceLevel(const Price &price)
{
    return levels.at(price);
}

template <typename Comparator>
void SideBook<Comparator>::removePriceLevel(const Price &price)
{
    auto it = levels.find(price);

    if (it != levels.end() && it->second.orders.empty())
        levels.erase(it);
}