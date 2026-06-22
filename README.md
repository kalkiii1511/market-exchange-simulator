# market-exchange-simulator
Electronic exchange simulator with FIX gateway, C++ matching engine, and Python market simulator.

exchange-simulator/
│
├── docs/
│   ├── architecture.md
│   ├── fix-spec.md
│   ├── api.md
│   └── diagrams/
│
├── docker/
│   ├── docker-compose.yml
│   └── monitoring/
│
├── scripts/
│   ├── build.sh
│   ├── run.sh
│   └── benchmark.sh
│
├── simulator/                 # Python
│   ├── traders/
│   │   ├── random_trader.py
│   │   ├── market_maker.py
│   │   └── momentum_trader.py
│   │
│   ├── strategies/
│   │   ├── mean_reversion.py
│   │   └── trend_following.py
│   │
│   ├── analytics/
│   │   ├── latency.py
│   │   ├── throughput.py
│   │   └── reports.py
│   │
│   ├── order_generator.py
│   ├── market_simulator.py
│   └── requirements.txt
│
├── gateway/                   # Go
│   ├── cmd/
│   │   └── gateway/
│   │       └── main.go
│   │
│   ├── internal/
│   │   ├── tcp/
│   │   │   ├── server.go
│   │   │   └── connection.go
│   │   │
│   │   ├── fix/
│   │   │   ├── parser.go
│   │   │   ├── encoder.go
│   │   │   ├── heartbeat.go
│   │   │   └── session.go
│   │   │
│   │   ├── grpc/
│   │   │   ├── client.go
│   │   │   └── proto/
│   │   │
│   │   └── marketdata/
│   │       └── publisher.go
│   │
│   ├── go.mod
│   └── go.sum
│
├── engine/                    # C++
│   ├── include/
│   │   ├── order.hpp
│   │   ├── trade.hpp
│   │   ├── order_book.hpp
│   │   ├── matching_engine.hpp
│   │   ├── risk_engine.hpp
│   │   └── types.hpp
│   │
│   ├── src/
│   │   ├── order_book.cpp
│   │   ├── matching_engine.cpp
│   │   ├── risk_engine.cpp
│   │   ├── grpc_server.cpp
│   │   └── main.cpp
│   │
│   ├── tests/
│   │   ├── order_book_test.cpp
│   │   ├── matching_test.cpp
│   │   └── risk_test.cpp
│   │
│   ├── benchmarks/
│   │   └── latency_benchmark.cpp
│   │
│   └── CMakeLists.txt
│
├── storage/
│   ├── schema.sql
│   ├── migrations/
│   └── seed.sql
│
├── proto/
│   └── exchange.proto
│
├── monitoring/
│   ├── prometheus.yml
│   └── grafana/
│
├── tests/
│   ├── integration/
│   └── load/
│
├── README.md
└── LICENSE
