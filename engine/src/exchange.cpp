#include "exchange.hpp"

Exchange::Exchange(RiskLimits riskLimits) : riskEngine(riskLimits) {}

ExecutionResult Exchange::submitOrder(std::unique_ptr<Order> order)
{
    if (!order)
        return {false, "Order must not be null.", {}};

    OrderId orderId = order->getId();
    std::string symbol = order->getSymbol();
    RiskResult riskResult = riskEngine.validate(
        *order, seenOrderIds.find(orderId) != seenOrderIds.end());
    if (!riskResult.accepted)
        return {false, riskResult.reason, {}};

    seenOrderIds.insert(orderId);
    OrderBook &book = books[symbol];
    std::vector<Trade> trades = matchingEngine.execute(book, std::move(order));

    for (const Trade &trade : trades)
    {
        if (!book.containsOrder(trade.buyId))
            orderSymbols.erase(trade.buyId);
        if (!book.containsOrder(trade.sellId))
            orderSymbols.erase(trade.sellId);
    }

    if (book.containsOrder(orderId))
        orderSymbols[orderId] = symbol;

    return {true, {}, std::move(trades)};
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
    return replaceOrder(orderId, newPrice, newQuantity).accepted;
}

ExecutionResult Exchange::replaceOrder(
    OrderId orderId,
    Price newPrice,
    Quantity newQuantity)
{
    auto symbolIt = orderSymbols.find(orderId);
    if (symbolIt == orderSymbols.end())
        return {false, "Order was not found.", {}};

    auto bookIt = books.find(symbolIt->second);
    if (bookIt == books.end())
        return {false, "Order book was not found.", {}};

    OrderLocation &location = bookIt->second.getOrderLocation(orderId);
    const Order &oldOrder = **location.orderIterator;
    auto replacement = std::make_unique<Order>(
        orderId,
        oldOrder.getSide(),
        newQuantity,
        oldOrder.getSymbol(),
        oldOrder.getType(),
        newPrice);

    RiskResult riskResult = riskEngine.validate(*replacement, false);
    if (!riskResult.accepted)
        return {false, riskResult.reason, {}};

    OrderBook &book = bookIt->second;
    const std::string symbol = symbolIt->second;
    book.cancelOrder(orderId);
    orderSymbols.erase(symbolIt);

    std::vector<Trade> trades = matchingEngine.execute(book, std::move(replacement));
    for (const Trade &trade : trades)
    {
        if (!book.containsOrder(trade.buyId))
            orderSymbols.erase(trade.buyId);
        if (!book.containsOrder(trade.sellId))
            orderSymbols.erase(trade.sellId);
    }

    if (book.containsOrder(orderId))
        orderSymbols[orderId] = symbol;

    return {true, {}, std::move(trades)};
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

RiskEngine &Exchange::getRiskEngine()
{
    return riskEngine;
}

const RiskEngine &Exchange::getRiskEngine() const
{
    return riskEngine;
}

void Exchange::clear()
{
    books.clear();
    orderSymbols.clear();
    seenOrderIds.clear();
}
