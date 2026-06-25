#include "exchange.hpp"

void Exchange::submitOrder(std::unique_ptr<Order> order)
{
    OrderId orderId = order->getId();
    std::string symbol = order->getSymbol();

    books[symbol].addOrder(std::move(order));
    orderSymbols[orderId] = symbol;
}

bool Exchange::cancelOrder(OrderId orderId)
{
    auto symbolIt = orderSymbols.find(orderId);
    if (symbolIt == orderSymbols.end())
        return false;

    auto bookIt = books.find(symbolIt->second);
    if (bookIt == books.end())
        return false;

    bool cancelled = bookIt->second.cancelOrder(orderId);
    if (cancelled)
        orderSymbols.erase(symbolIt);

    return cancelled;
}

bool Exchange::modifyOrder(OrderId orderId, Price newPrice, Quantity newQuantity)
{
    auto symbolIt = orderSymbols.find(orderId);
    if (symbolIt == orderSymbols.end())
        return false;

    auto bookIt = books.find(symbolIt->second);
    if (bookIt == books.end())
        return false;

    return bookIt->second.modifyOrder(orderId, newPrice, newQuantity);
}

bool Exchange::containsOrder(OrderId orderId) const
{
    return orderSymbols.find(orderId) != orderSymbols.end();
}

OrderBook &Exchange::getOrderBook(const std::string &symbol)
{
    return books.at(symbol);
}

const OrderBook &Exchange::getOrderBook(const std::string &symbol) const
{
    return books.at(symbol);
}

bool Exchange::containsSymbol(const std::string &symbol) const
{
    return books.find(symbol) != books.end();
}

void Exchange::clear()
{
    books.clear();
    orderSymbols.clear();
}
