#ifndef EXCHANGE_BRIDGE_H
#define EXCHANGE_BRIDGE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int accepted;
    uint32_t filled_quantity;
    uint32_t leaves_quantity;
    double average_price;
    uint32_t trade_count;
    char reason[256];
} EngineSubmitResult;

EngineSubmitResult engine_submit_order(
    uint64_t id,
    const char *side,
    uint32_t quantity,
    const char *symbol,
    double price);

#ifdef __cplusplus
}
#endif

#endif
