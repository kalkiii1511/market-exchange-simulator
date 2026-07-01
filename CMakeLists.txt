cmake_minimum_required(VERSION 3.20)
project(market_exchange_simulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(exchange_engine STATIC
    engine/src/exchange.cpp
    engine/src/matchingEngine.cpp
    engine/src/order.cpp
    engine/src/orderBook.cpp
    engine/src/riskEngine.cpp
)
target_include_directories(exchange_engine PUBLIC engine/include)

add_executable(engine_test engine/tests/engine_test.cpp)
target_link_libraries(engine_test PRIVATE exchange_engine)

enable_testing()
add_test(NAME engine_test COMMAND engine_test)
