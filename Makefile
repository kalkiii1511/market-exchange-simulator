.PHONY: build test clean

build:
	cmake -S . -B build
	cmake --build build
	cd gateway && GOCACHE="$(CURDIR)/build/go-cache" go build -o ../build/fix-gateway ./cmd/gateway

test:
	cmake -S . -B build
	cmake --build build
	ctest --test-dir build --output-on-failure
	cd gateway && GOCACHE="$(CURDIR)/build/go-cache" go test ./...

clean:
	cmake -E remove_directory build
