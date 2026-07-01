package engine

/*
#cgo CXXFLAGS: -std=c++17 -I${SRCDIR}/../../../engine/include
#cgo darwin LDFLAGS: ${SRCDIR}/../../../build/libexchange_engine.a -lc++
#cgo linux LDFLAGS: ${SRCDIR}/../../../build/libexchange_engine.a -lstdc++
#include <stdlib.h>
#include "bridge.h"
*/
import "C"

import (
	"context"
	"errors"
	"unsafe"
)

type NativeClient struct{}

func NewNativeClient() *NativeClient { return &NativeClient{} }

func (client *NativeClient) Submit(ctx context.Context, order NewOrder) (SubmitResult, error) {
	if err := ctx.Err(); err != nil {
		return SubmitResult{}, err
	}
	side := C.CString(order.Side)
	symbol := C.CString(order.Symbol)
	defer C.free(unsafe.Pointer(side))
	defer C.free(unsafe.Pointer(symbol))

	result := C.engine_submit_order(
		C.uint64_t(order.ID), side, C.uint32_t(order.Quantity), symbol, C.double(order.Price),
	)
	response := SubmitResult{
		Accepted:       result.accepted != 0,
		Reason:         C.GoString(&result.reason[0]),
		FilledQuantity: uint32(result.filled_quantity),
		LeavesQuantity: uint32(result.leaves_quantity),
		AveragePrice:   float64(result.average_price),
		TradeCount:     uint32(result.trade_count),
	}
	if !response.Accepted && response.Reason == "" {
		return response, errors.New("engine rejected the order without a reason")
	}
	return response, nil
}
