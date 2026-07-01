package main

import (
	"context"
	"flag"
	"log/slog"
	"os"
	"os/signal"
	"syscall"

	"market-exchange-simulator/gateway/internal/engine"
	"market-exchange-simulator/gateway/internal/gateway"
)

func main() {
	address := flag.String("listen", ":9878", "TCP address for the FIX gateway")
	flag.Parse()

	logger := slog.New(slog.NewTextHandler(os.Stdout, nil))
	ctx, stop := signal.NotifyContext(context.Background(), os.Interrupt, syscall.SIGTERM)
	defer stop()

	server := gateway.Server{
		Address: *address,
		Handler: gateway.NewHandler(engine.NewNativeClient()),
		Logger:  logger,
	}
	if err := server.ListenAndServe(ctx); err != nil {
		logger.Error("FIX gateway stopped", "error", err)
		os.Exit(1)
	}
}
