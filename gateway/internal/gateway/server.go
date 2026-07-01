package gateway

import (
	"context"
	"errors"
	"io"
	"log/slog"
	"net"

	"market-exchange-simulator/gateway/internal/fix"
)

type Server struct {
	Address string
	Handler *Handler
	Logger  *slog.Logger
}

func (server *Server) ListenAndServe(ctx context.Context) error {
	listener, err := net.Listen("tcp", server.Address)
	if err != nil {
		return err
	}
	defer listener.Close()
	go func() {
		<-ctx.Done()
		listener.Close()
	}()
	server.Logger.Info("FIX gateway listening", "address", listener.Addr().String())

	for {
		connection, acceptErr := listener.Accept()
		if acceptErr != nil {
			if ctx.Err() != nil {
				return nil
			}
			return acceptErr
		}
		go server.serveConnection(ctx, connection)
	}
}

func (server *Server) serveConnection(ctx context.Context, connection net.Conn) {
	defer connection.Close()
	logger := server.Logger.With("remote", connection.RemoteAddr().String())
	logger.Info("FIX client connected")
	defer logger.Info("FIX client disconnected")

	decoder := fix.Decoder{}
	session := Session{}
	buffer := make([]byte, 16*1024)
	for {
		count, err := connection.Read(buffer)
		if count > 0 {
			messages, decodeErr := decoder.Write(buffer[:count])
			if decodeErr != nil {
				logger.Warn("invalid FIX frame", "error", decodeErr)
				return
			}
			for _, message := range messages {
				response := server.Handler.Handle(ctx, &session, message)
				if writeErr := writeAll(connection, response); writeErr != nil {
					logger.Warn("failed to write FIX response", "error", writeErr)
					return
				}
			}
		}
		if err != nil {
			if !errors.Is(err, io.EOF) {
				logger.Warn("FIX connection read failed", "error", err)
			}
			return
		}
	}
}

func writeAll(writer io.Writer, data []byte) error {
	for len(data) != 0 {
		count, err := writer.Write(data)
		if err != nil {
			return err
		}
		data = data[count:]
	}
	return nil
}
