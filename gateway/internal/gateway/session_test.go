package gateway

import (
	"context"
	"testing"

	"market-exchange-simulator/gateway/internal/engine"
	"market-exchange-simulator/gateway/internal/fix"
)

type fakeEngine struct {
	order engine.NewOrder
}

func (fake *fakeEngine) Submit(_ context.Context, order engine.NewOrder) (engine.SubmitResult, error) {
	fake.order = order
	return engine.SubmitResult{Accepted: true, LeavesQuantity: order.Quantity}, nil
}

func TestLogonThenNewOrder(t *testing.T) {
	client := &fakeEngine{}
	handler := NewHandler(client)
	session := &Session{}

	logon := handler.Handle(context.Background(), session, fix.Message{
		35: "A", 49: "TRADER", 56: "EXCHANGE",
	})
	logonMessage := decodeResponse(t, logon)
	if logonMessage[35] != "A" || !session.LoggedOn {
		t.Fatalf("unexpected logon response: %#v", logonMessage)
	}

	response := handler.Handle(context.Background(), session, fix.Message{
		35: "D", 11: "client-1", 55: "ACME", 54: "1", 38: "25", 40: "2", 44: "101.50",
	})
	message := decodeResponse(t, response)
	if message[35] != "8" || message[39] != "0" || message[11] != "client-1" {
		t.Fatalf("unexpected execution report: %#v", message)
	}
	if client.order.Side != "BUY" || client.order.Quantity != 25 || client.order.Symbol != "ACME" {
		t.Fatalf("unexpected engine order: %#v", client.order)
	}
}

func TestOrderBeforeLogonIsRejected(t *testing.T) {
	handler := NewHandler(&fakeEngine{})
	response := handler.Handle(context.Background(), &Session{}, fix.Message{35: "D"})
	message := decodeResponse(t, response)
	if message[35] != "3" {
		t.Fatalf("expected session reject, got %#v", message)
	}
}

func decodeResponse(t *testing.T, response []byte) fix.Message {
	t.Helper()
	decoder := fix.Decoder{}
	messages, err := decoder.Write(response)
	if err != nil || len(messages) != 1 {
		t.Fatalf("decode response = %#v, %v", messages, err)
	}
	return messages[0]
}
