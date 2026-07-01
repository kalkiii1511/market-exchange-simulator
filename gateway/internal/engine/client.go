package engine

import "context"

type NewOrder struct {
	ID       uint64
	Side     string
	Quantity uint32
	Symbol   string
	Price    float64
}

type SubmitResult struct {
	Accepted       bool
	Reason         string
	FilledQuantity uint32
	LeavesQuantity uint32
	AveragePrice   float64
	TradeCount     uint32
}

type Client interface {
	Submit(context.Context, NewOrder) (SubmitResult, error)
}
