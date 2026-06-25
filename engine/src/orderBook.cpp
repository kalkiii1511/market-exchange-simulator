#include "orderBook.hpp"

#include <iterator>

void OrderBook::addOrder(std::unique_ptr<Order> order)
{
    OrderId id = order->getId();
    std::string side = order->getSide();
    Price price = order->getPrice();

    if (side == "BUY")
    {
        bids.addOrder(std::move(order));
        PriceLevel &level = bids.getPriceLevel(price);
        addOrderIndex(id, {side, &level, std::prev(level.orders.end())});
    }
    else
    {
        asks.addOrder(std::move(order));
        PriceLevel &level = asks.getPriceLevel(price);
        addOrderIndex(id, {side, &level, std::prev(level.orders.end())});
    }
}

bool OrderBook::cancelOrder(OrderId id)
{
    auto idx = orderIndex.find(id);

    if (idx == orderIndex.end())
        return false;

    OrderLocation location = idx->second;

    location.level->orders.erase(location.orderIterator);

    if (location.level->orders.empty())
    {
        if (location.side == "BUY")
            bids.removePriceLevel(location.level->price);
        else
            asks.removePriceLevel(location.level->price);
    }

    orderIndex.erase(idx);

    return true;
}

bool OrderBook::modifyOrder(OrderId orderId, Price newPrice, Quantity newQuantity)
{
    auto it = orderIndex.find(orderId);
    if (it == orderIndex.end())
        return false;

    Order oldOrder = **(it->second.orderIterator);

    cancelOrder(orderId);

    auto newOrder = std::make_unique<Order>(
        orderId,
        oldOrder.getSide(),
        newQuantity,
        oldOrder.getSymbol(),
        oldOrder.getType(),
        newPrice
    );

    addOrder(std::move(newOrder));

    return true;
}

bool OrderBook::containsOrder(OrderId orderId) const{
    return orderIndex.find(orderId) != orderIndex.end();
}

PriceLevel &OrderBook::bestBid()
{
    return bids.bestPriceLevel();
}

PriceLevel &OrderBook::bestAsk()
{
    return asks.bestPriceLevel();
}

OrderLocation &OrderBook::getOrderLocation(OrderId orderId)
{
    return orderIndex.at(orderId);
}

void OrderBook::removeOrderIndex(OrderId orderId)
{
    orderIndex.erase(orderId);
}

void OrderBook::addOrderIndex(OrderId orderId, const OrderLocation &location)
{
    orderIndex[orderId] = location;
}

bool OrderBook::empty() const
{
    return bids.empty() && asks.empty();
}

void OrderBook::clear()
{
    bids = BidBook{};
    asks = AskBook{};
    orderIndex.clear();
}
