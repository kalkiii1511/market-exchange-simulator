#include "bridge.h"

#include "exchange.hpp"

#include <algorithm>
#include <cstring>
#include <memory>
#include <mutex>

namespace {

Exchange exchange;
std::mutex exchangeMutex;

void copyReason(char *destination, const std::string &reason)
{
    const std::size_t length = std::min(reason.size(), std::size_t{255});
    std::memcpy(destination, reason.data(), length);
    destination[length] = '\0';
}

} // namespace

extern "C" EngineSubmitResult engine_submit_order(
    uint64_t id,
    const char *side,
    uint32_t quantity,
    const char *symbol,
    double price)
{
    EngineSubmitResult response{};
    if (side == nullptr || symbol == nullptr)
    {
        copyReason(response.reason, "Side and symbol are required.");
        return response;
    }

    std::lock_guard<std::mutex> lock(exchangeMutex);
    auto order = std::make_unique<Order>(
        id, side, quantity, symbol, "LIMIT", Price{price});
    ExecutionResult result = exchange.submitOrder(std::move(order));
    response.accepted = result.accepted ? 1 : 0;
    if (!result.accepted)
    {
        copyReason(response.reason, result.rejectionReason);
        return response;
    }

    double tradedNotional = 0.0;
    for (const Trade &trade : result.trades)
    {
        if (trade.buyId == id || trade.sellId == id)
        {
            response.filled_quantity += trade.quantity;
            tradedNotional += trade.price.amount * trade.quantity;
            ++response.trade_count;
        }
    }
    response.leaves_quantity = quantity - response.filled_quantity;
    if (response.filled_quantity != 0)
        response.average_price = tradedNotional / response.filled_quantity;
    return response;
}
