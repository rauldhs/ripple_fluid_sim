build:
    cmake --build build

run: build
    ./build/src/ripple

test: build
    GTEST_COLOR=yes ctest --test-dir build --output-on-failure

clean:
    rm -rf build/*

configure:
    cmake -S . -B build
