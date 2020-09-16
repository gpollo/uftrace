#include <cstdlib>
#include <limits>

#include "catch.hpp"
#include "libexpr/impl/process_memory.hpp"

template <typename T>
static void test_memory_accesses(libexpr::process_memory& memory) {
    for (int i = 0; i < 10; i++) {
        T expected = (((double)std::rand()) / ((double)RAND_MAX + 1)) * std::numeric_limits<T>::max();
        T result;

        uint64_t addr = (uint64_t)&expected;

        REQUIRE(memory.read(addr, result));
        REQUIRE(result == expected);
    }
}

TEST_CASE("reading from process memory should work", "[libexpr::process_memory]") {
    libexpr::process_memory memory;

    SECTION("reading uint8_t should work") {
        test_memory_accesses<uint8_t>(memory);
    }

    SECTION("reading uint16_t should work") {
        test_memory_accesses<uint16_t>(memory);
    }

    SECTION("reading uint32_t should work") {
        test_memory_accesses<uint32_t>(memory);
    }

    SECTION("reading uint64_t should work") {
        test_memory_accesses<uint64_t>(memory);
    }

    SECTION("reading int8_t should work") {
        test_memory_accesses<int8_t>(memory);
    }

    SECTION("reading int16_t should work") {
        test_memory_accesses<int16_t>(memory);
    }

    SECTION("reading int32_t should work") {
        test_memory_accesses<int32_t>(memory);
    }

    SECTION("reading int64_t should work") {
        test_memory_accesses<int64_t>(memory);
    }
}