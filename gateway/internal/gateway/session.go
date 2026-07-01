package gateway

import (
	"context"
	"fmt"
	"strconv"
	"sync/atomic"
	"time"

	"market-exchange-simulator/gateway/internal/engine"
	"market-exchange-simulator/gateway/internal/fix"
)

type Handler struct {
	engine      engine.Client
	nextOrderID atomic.Uint64
}

type Session struct {
	SenderCompID   string
	TargetCompID   string
	OutboundSeqNum uint64
	LoggedOn       bool
}

func NewHandler(client engine.Client) *Handler {
	handler := &Handler{engine: client}
	handler.nextOrderID.Store(1000)
	return handler
}

func (handler *Handler) Handle(ctx context.Context, session *Session, message fix.Message) []byte {
	if sender := message[49]; sender != "" {
		session.TargetCompID = sender
	}
	if target := message[56]; target != "" {
		session.SenderCompID = target
	}
	messageType, err := message.Require(35)
	if err != nil {
		return handler.reject(session, err.Error())
	}

	switch messageType {
	case "A":
		session.LoggedOn = true
		return handler.encode(session, "A", fix.Field{Tag: 98, Value: "0"}, fix.Field{Tag: 108, Value: "30"})
	case "0":
		return handler.encode(session, "0")
	case "D":
		if !session.LoggedOn {
			return handler.reject(session, "Logon is required before application messages.")
		}
		return handler.newOrder(ctx, session, message)
	default:
		return handler.reject(session, "Unsupported MsgType "+messageType+".")
	}
}

func (handler *Handler) newOrder(ctx context.Context, session *Session, message fix.Message) []byte {
	clOrdID, err := message.Require(11)
	if err != nil {
		return handler.reject(session, err.Error())
	}
	symbol, err := message.Require(55)
	if err != nil {
		return handler.orderReject(session, clOrdID, symbol, "", 0, 0, err.Error())
	}
	sideValue, err := message.Require(54)
	if err != nil {
		return handler.orderReject(session, clOrdID, symbol, "", 0, 0, err.Error())
	}
	side := map[string]string{"1": "BUY", "2": "SELL"}[sideValue]
	if side == "" {
		return handler.orderReject(session, clOrdID, symbol, sideValue, 0, 0, "Side must be 1 (buy) or 2 (sell).")
	}
	if orderType, _ := message.Require(40); orderType != "2" {
		return handler.orderReject(session, clOrdID, symbol, sideValue, 0, 0, "Only limit orders (OrdType=2) are supported.")
	}
	quantityValue, err := message.Require(38)
	if err != nil {
		return handler.orderReject(session, clOrdID, symbol, sideValue, 0, 0, err.Error())
	}
	quantity, parseErr := strconv.ParseUint(quantityValue, 10, 32)
	if parseErr != nil || quantity == 0 {
		return handler.orderReject(session, clOrdID, symbol, sideValue, 0, 0, "OrderQty must be a positive integer.")
	}
	priceValue, err := message.Require(44)
	if err != nil {
		return handler.orderReject(session, clOrdID, symbol, sideValue, uint32(quantity), 0, err.Error())
	}
	price, parseErr := strconv.ParseFloat(priceValue, 64)
	if parseErr != nil || price <= 0 {
		return handler.orderReject(session, clOrdID, symbol, sideValue, uint32(quantity), 0, "Price must be positive.")
	}

	orderID := handler.nextOrderID.Add(1)
	result, submitErr := handler.engine.Submit(ctx, engine.NewOrder{
		ID: orderID, Side: side, Quantity: uint32(quantity), Symbol: symbol, Price: price,
	})
	if submitErr != nil {
		return handler.orderReject(session, clOrdID, symbol, sideValue, uint32(quantity), price, submitErr.Error())
	}
	if !result.Accepted {
		return handler.orderReject(session, clOrdID, symbol, sideValue, uint32(quantity), price, result.Reason)
	}

	status, executionType := "0", "0"
	if result.FilledQuantity != 0 {
		status, executionType = "1", "F"
		if result.LeavesQuantity == 0 {
			status = "2"
		}
	}
	return handler.encode(session, "8",
		fix.Field{Tag: 37, Value: strconv.FormatUint(orderID, 10)},
		fix.Field{Tag: 11, Value: clOrdID},
		fix.Field{Tag: 17, Value: fmt.Sprintf("%d-%d", orderID, result.TradeCount)},
		fix.Field{Tag: 150, Value: executionType},
		fix.Field{Tag: 39, Value: status},
		fix.Field{Tag: 55, Value: symbol},
		fix.Field{Tag: 54, Value: sideValue},
		fix.Field{Tag: 38, Value: strconv.FormatUint(quantity, 10)},
		fix.Field{Tag: 44, Value: strconv.FormatFloat(price, 'f', -1, 64)},
		fix.Field{Tag: 14, Value: strconv.FormatUint(uint64(result.FilledQuantity), 10)},
		fix.Field{Tag: 151, Value: strconv.FormatUint(uint64(result.LeavesQuantity), 10)},
		fix.Field{Tag: 6, Value: strconv.FormatFloat(result.AveragePrice, 'f', -1, 64)},
	)
}

func (handler *Handler) orderReject(
	session *Session, clOrdID, symbol, side string, quantity uint32, price float64, reason string,
) []byte {
	return handler.encode(session, "8",
		fix.Field{Tag: 37, Value: "NONE"}, fix.Field{Tag: 11, Value: clOrdID},
		fix.Field{Tag: 17, Value: "NONE"}, fix.Field{Tag: 150, Value: "8"},
		fix.Field{Tag: 39, Value: "8"}, fix.Field{Tag: 55, Value: symbol},
		fix.Field{Tag: 54, Value: side}, fix.Field{Tag: 38, Value: strconv.FormatUint(uint64(quantity), 10)},
		fix.Field{Tag: 44, Value: strconv.FormatFloat(price, 'f', -1, 64)},
		fix.Field{Tag: 58, Value: reason},
	)
}

func (handler *Handler) reject(session *Session, reason string) []byte {
	return handler.encode(session, "3", fix.Field{Tag: 58, Value: reason})
}

func (handler *Handler) encode(session *Session, messageType string, fields ...fix.Field) []byte {
	session.OutboundSeqNum++
	header := []fix.Field{
		{Tag: 35, Value: messageType},
		{Tag: 49, Value: defaultValue(session.SenderCompID, "EXCHANGE")},
		{Tag: 56, Value: defaultValue(session.TargetCompID, "CLIENT")},
		{Tag: 34, Value: strconv.FormatUint(session.OutboundSeqNum, 10)},
		{Tag: 52, Value: time.Now().UTC().Format("20060102-15:04:05.000")},
	}
	return fix.Encode("FIX.4.4", append(header, fields...)...)
}

func defaultValue(value, fallback string) string {
	if value == "" {
		return fallback
	}
	return value
}
