# Market Exchange Simulator

Market Exchange Simulator is an in-memory electronic exchange built to explore
order entry, risk validation, price-time priority, and trade execution.

Clients connect to a Go TCP gateway using FIX 4.4 messages. The gateway validates
and translates each request, then submits it to the existing C++ matching engine.
The current implementation runs both components in one process through a small
cgo adapter.

## Architecture

```text
FIX client
    |
    | TCP / FIX 4.4
    v
Go gateway
    |-- message framing and checksum validation
    |-- FIX session handling
    |-- order validation and translation
    v
engine.Client interface
    |
    | cgo / C ABI
    v
C++ exchange
    |-- risk engine
    |-- matching engine
    `-- per-symbol order books
```

TCP connections are handled concurrently by Go. Access to the C++ exchange is
serialized because the current matching engine and order books are not designed
for concurrent mutation.

## Current features

- Streaming TCP server that handles fragmented and coalesced FIX frames
- FIX `BodyLength(9)` and `CheckSum(10)` validation
- FIX Logon, Heartbeat, and NewOrderSingle handling
- Limit buy and sell orders
- Order quantity, price, side, duplicate ID, and notional risk checks
- Per-symbol order books
- Price-time priority matching
- Partial fills and resting order remainders
- ExecutionReport responses containing fill and remaining quantities
- Concurrent client connections
- Graceful shutdown on `SIGINT` or `SIGTERM`

## Order flow

1. A client opens a TCP connection and sends a valid `Logon(35=A)` message.
2. The gateway returns a Logon acknowledgement.
3. The client sends a `NewOrderSingle(35=D)` message.
4. The gateway validates the required FIX fields and converts FIX values into an
   engine order.
5. The C++ risk engine accepts or rejects the order.
6. Accepted orders execute against the opposite side of the symbol's book.
7. The gateway returns an `ExecutionReport(35=8)` with the resulting status.

All FIX fields must use the standard SOH byte (`0x01`) as the delimiter.

## Supported FIX messages

| MsgType | Name | Direction |
| --- | --- | --- |
| `A` | Logon | Client to gateway and acknowledgement |
| `0` | Heartbeat | Client to gateway and response |
| `D` | NewOrderSingle | Client to gateway |
| `8` | ExecutionReport | Gateway to client |
| `3` | Reject | Gateway to client |

A NewOrderSingle currently supports limit orders only and requires:

| Tag | Field | Accepted value |
| --- | --- | --- |
| `11` | ClOrdID | Non-empty client order ID |
| `55` | Symbol | Non-empty symbol |
| `54` | Side | `1` for buy, `2` for sell |
| `38` | OrderQty | Positive 32-bit integer |
| `40` | OrdType | `2` for limit |
| `44` | Price | Positive number |

The frame must also contain valid standard header fields, `BodyLength(9)`, and
`CheckSum(10)`.

## Repository layout

```text
.
|-- engine/                    C++ exchange and tests
|   |-- include/               Public engine headers
|   |-- src/                   Risk, order-book, and matching logic
|   `-- tests/                 C++ engine tests
|-- gateway/                   Go FIX gateway
|   |-- cmd/gateway/           Executable entry point
|   `-- internal/
|       |-- engine/            Engine interface and cgo bridge
|       |-- fix/               FIX framing, decoding, and encoding
|       `-- gateway/           TCP server and FIX session handling
|-- CMakeLists.txt             C++ build configuration
|-- Makefile                   Combined Go and C++ commands
`-- direction.md              Longer-term project direction
```

## Requirements

- Go 1.22 or newer with cgo enabled
- CMake 3.20 or newer
- A C++17 compiler
- Make

## Build and run

Build the C++ static library, tests, and Go gateway:

```sh
make build
```

Start the gateway on the default TCP port:

```sh
./build/fix-gateway
```

Use a different listening address when required:

```sh
./build/fix-gateway -listen 127.0.0.1:9000
```

The default address is `:9878`.

## Tests

Run the complete C++ and Go test suites:

```sh
make test
```

The tests cover matching behavior, risk rejection, FIX stream framing, checksum
validation, FIX session handling, and the Go-to-C++ native bridge.

## Current limitations

- State is held in memory and is lost when the process exits.
- Only limit NewOrderSingle requests are supported.
- Cancel, replace, logout, resend, and sequence recovery are not implemented.
- Session sequence numbers are emitted but incoming sequence numbers are not yet
  enforced.
- One mutex serializes access to the entire C++ exchange.
- The Go and C++ components currently share a process through cgo.

The engine is exposed behind a Go `engine.Client` interface so the cgo adapter can
later be replaced with an out-of-process transport such as gRPC. See
[direction.md](direction.md) for the broader planned system.
