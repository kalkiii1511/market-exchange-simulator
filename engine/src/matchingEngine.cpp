#include "matchingEngine.hpp"

#include <algorithm>
#include <chrono>
#include <stdexcept>

std::vector<Trade> MatchingEngine::execute(
    OrderBook &book,
    std::unique_ptr<Order> incomingOrder)
{
    if (!incomingOrder)
        throw std::invalid_argument("Incoming order must not be null.");

    std::vector<Trade> trades;
    const bool isBuy = incomingOrder->getSide() == "BUY";

    while (!incomingOrder->isFilled())
    {
        PriceLevel *oppositeLevel = nullptr;

        try
        {
            oppositeLevel = isBuy ? &book.bestAsk() : &book.bestBid();
        }
        catch (const std::runtime_error &)
        {
            break;
        }

        const bool crosses = isBuy
            ? incomingOrder->getPrice().amount >= oppositeLevel->price.amount
            : incomingOrder->getPrice().amount <= oppositeLevel->price.amount;
        if (!crosses)
            break;

        Order &restingOrder = *oppositeLevel->orders.front();
        const uint32_t fillQuantity = std::min(
            incomingOrder->getRemaining(), restingOrder.getRemaining());
        const Price tradePrice = restingOrder.getPrice();

        incomingOrder->fill(fillQuantity);
        restingOrder.fill(fillQuantity);

        trades.push_back(isBuy
            ? createTrade(*incomingOrder, restingOrder, fillQuantity, tradePrice)
            : createTrade(restingOrder, *incomingOrder, fillQuantity, tradePrice));

        if (restingOrder.isFilled())
            book.cancelOrder(restingOrder.getId());
    }

    if (!incomingOrder->isFilled())
        book.addOrder(std::move(incomingOrder));

    return trades;
}

Trade MatchingEngine::createTrade(
    const Order &buyOrder,
    const Order &sellOrder,
    uint32_t quantity,
    Price price)
{
    return {
        nextTradeId++,
        buyOrder.getSymbol(),
        buyOrder.getId(),
        sellOrder.getId(),
        quantity,
        price,
        std::chrono::system_clock::now()
    };
}
