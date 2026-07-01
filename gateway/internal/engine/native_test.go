package engine

import (
	"context"
	"testing"
)

func TestNativeClientSubmitsToMatchingEngine(t *testing.T) {
	client := NewNativeClient()
	buy, err := client.Submit(context.Background(), NewOrder{
		ID: 900001, Side: "BUY", Quantity: 10, Symbol: "NATIVE_TEST", Price: 100,
	})
	if err != nil || !buy.Accepted || buy.LeavesQuantity != 10 {
		t.Fatalf("buy = %#v, %v", buy, err)
	}
	sell, err := client.Submit(context.Background(), NewOrder{
		ID: 900002, Side: "SELL", Quantity: 4, Symbol: "NATIVE_TEST", Price: 99,
	})
	if err != nil || !sell.Accepted {
		t.Fatalf("sell = %#v, %v", sell, err)
	}
	if sell.FilledQuantity != 4 || sell.LeavesQuantity != 0 || sell.AveragePrice != 100 {
		t.Fatalf("unexpected execution: %#v", sell)
	}
}
