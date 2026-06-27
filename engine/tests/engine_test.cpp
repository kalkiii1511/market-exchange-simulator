#include <cassert>
#include <cmath>
#include <memory>
#include <string>
#include "exchange.hpp"

namespace {

std::unique_ptr<Order> makeOrder(
    uint64_t id,
    const std::string &side,
    uint32_t quantity,
    const std::string &symbol,
    double price)
{
    return std::make_unique<Order>(
        id, side, quantity, symbol, "LIMIT", Price{price});
}

const Order &findOrder(const Exchange &exchange, const std::string &symbol, uint64_t id)
{
    const OrderLocation &location = exchange.getOrderBook(symbol).getOrderLocation(id);
    return **location.orderIterator;
}

void testPartialFillAtMakerPrice()
{
    Exchange exchange;

    ExecutionResult resting = exchange.submitOrder(
        makeOrder(1, "BUY", 10, "ACME", 100.0));
    assert(resting.accepted && resting.trades.empty());

    ExecutionResult execution = exchange.submitOrder(
        makeOrder(2, "SELL", 4, "ACME", 99.0));
    assert(execution.accepted && execution.trades.size() == 1);
    assert(execution.trades[0].buyId == 1);
    assert(execution.trades[0].sellId == 2);
    assert(execution.trades[0].quantity == 4);
    assert(std::abs(execution.trades[0].price.amount - 100.0) < 1e-9);
    assert(exchange.containsOrder(1));
    assert(!exchange.containsOrder(2));
    assert(findOrder(exchange, "ACME", 1).getRemaining() == 6);
}

void testRemainderRestsOnBook()
{
    Exchange exchange;
    exchange.submitOrder(makeOrder(10, "BUY", 3, "ACME", 100.0));

    ExecutionResult execution = exchange.submitOrder(
        makeOrder(11, "SELL", 8, "ACME", 100.0));
    assert(execution.trades.size() == 1);
    assert(!exchange.containsOrder(10));
    assert(exchange.containsOrder(11));
    assert(findOrder(exchange, "ACME", 11).getRemaining() == 5);
}

void testPriceTimePriority()
{
    Exchange exchange;
    exchange.submitOrder(makeOrder(20, "SELL", 5, "ACME", 101.0));
    exchange.submitOrder(makeOrder(21, "SELL", 5, "ACME", 101.0));

    ExecutionResult execution = exchange.submitOrder(
        makeOrder(22, "BUY", 8, "ACME", 102.0));
    assert(execution.trades.size() == 2);
    assert(execution.trades[0].sellId == 20);
    assert(execution.trades[0].quantity == 5);
    assert(execution.trades[1].sellId == 21);
    assert(execution.trades[1].quantity == 3);
    assert(findOrder(exchange, "ACME", 21).getRemaining() == 2);
}

void testRiskRejections()
{
    Exchange exchange(RiskLimits{100, 5'000.0});

    ExecutionResult invalidSide = exchange.submitOrder(
        makeOrder(30, "HOLD", 1, "ACME", 10.0));
    assert(!invalidSide.accepted);

    ExecutionResult excessiveQuantity = exchange.submitOrder(
        makeOrder(31, "BUY", 101, "ACME", 10.0));
    assert(!excessiveQuantity.accepted);

    ExecutionResult valid = exchange.submitOrder(
        makeOrder(32, "BUY", 10, "ACME", 10.0));
    assert(valid.accepted);

    ExecutionResult duplicate = exchange.submitOrder(
        makeOrder(32, "BUY", 10, "ACME", 10.0));
    assert(!duplicate.accepted);

    exchange.submitOrder(makeOrder(33, "SELL", 10, "ACME", 10.0));
    assert(!exchange.containsOrder(32));
    ExecutionResult reusedFilledId = exchange.submitOrder(
        makeOrder(32, "BUY", 10, "ACME", 10.0));
    assert(!reusedFilledId.accepted);
}

void testReplacementUsesMatchingAndRisk()
{
    Exchange exchange(RiskLimits{100, 50'000.0});
    exchange.submitOrder(makeOrder(40, "SELL", 5, "ACME", 105.0));
    exchange.submitOrder(makeOrder(41, "BUY", 5, "ACME", 100.0));

    ExecutionResult rejected = exchange.replaceOrder(41, Price{100.0}, 101);
    assert(!rejected.accepted);
    assert(exchange.containsOrder(41));

    ExecutionResult replaced = exchange.replaceOrder(41, Price{105.0}, 5);
    assert(replaced.accepted);
    assert(replaced.trades.size() == 1);
    assert(replaced.trades[0].buyId == 41);
    assert(replaced.trades[0].sellId == 40);
    assert(!exchange.containsOrder(40));
    assert(!exchange.containsOrder(41));
}

} // namespace

int main()
{
    testPartialFillAtMakerPrice();
    testRemainderRestsOnBook();
    testPriceTimePriority();
    testRiskRejections();
    testReplacementUsesMatchingAndRisk();
}
