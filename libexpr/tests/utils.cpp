#include <limits>

#include "catch.hpp"
#include "libexpr/utils.hpp"

TEST_CASE("int128_t helper functions work", "[utils::int128_t]") {
    SECTION("nonstd::to_string_binary(1) works") {
        std::string expected("0b");
        expected +=
            "00000000"
            "00000000"
            "00000000"
            "00000000";
        expected +=
            "00000000"
            "00000000"
            "00000000"
            "00000000";
        expected +=
            "00000000"
            "00000000"
            "00000000"
            "00000000";
        expected +=
            "00000000"
            "00000000"
            "00000000"
            "00000001";
        REQUIRE(nonstd::to_string_binary(1) == expected);
    }

    SECTION("nonstd::to_string_binary(-1) works") {
        std::string expected("0b");
        expected +=
            "11111111"
            "11111111"
            "11111111"
            "11111111";
        expected +=
            "11111111"
            "11111111"
            "11111111"
            "11111111";
        expected +=
            "11111111"
            "11111111"
            "11111111"
            "11111111";
        expected +=
            "11111111"
            "11111111"
            "11111111"
            "11111111";
        REQUIRE(nonstd::to_string_binary(-1) == expected);
    }
}

TEST_CASE("int64_t and uint64_t inside int128_t works as expected", "[utils::int128_t]") {
    SECTION("uint64_t(max) to int128_t pads with zero") {
        std::string expected("0b");
        expected +=
            "00000000"
            "00000000"
            "00000000"
            "00000000";
        expected +=
            "00000000"
            "00000000"
            "00000000"
            "00000000";
        expected +=
            "11111111"
            "11111111"
            "11111111"
            "11111111";
        expected +=
            "11111111"
            "11111111"
            "11111111"
            "11111111";

        uint64_t u64  = std::numeric_limits<uint64_t>::max();
        int128_t i128 = u64;

        REQUIRE(nonstd::to_string_binary(i128) == expected);
    }

    SECTION("int64_t(max) to int128_t pads with zero") {
        std::string expected("0b");
        expected +=
            "00000000"
            "00000000"
            "00000000"
            "00000000";
        expected +=
            "00000000"
            "00000000"
            "00000000"
            "00000000";
        expected +=
            "01111111"
            "11111111"
            "11111111"
            "11111111";
        expected +=
            "11111111"
            "11111111"
            "11111111"
            "11111111";

        int64_t i64   = std::numeric_limits<int64_t>::max();
        int128_t i128 = i64;

        REQUIRE(nonstd::to_string_binary(i128) == expected);
    }

    SECTION("int64_t(uint64_t(max)) to int128_t pads with one") {
        std::string expected("0b");
        expected +=
            "11111111"
            "11111111"
            "11111111"
            "11111111";
        expected +=
            "11111111"
            "11111111"
            "11111111"
            "11111111";
        expected +=
            "11111111"
            "11111111"
            "11111111"
            "11111111";
        expected +=
            "11111111"
            "11111111"
            "11111111"
            "11111111";

        uint64_t u64  = std::numeric_limits<uint64_t>::max();
        int64_t i64   = u64;
        int128_t i128 = i64;

        REQUIRE(nonstd::to_string_binary(i128) == expected);
    }

    SECTION("uint64_t(int64_t(max)) to int128_t pads with zero") {
        std::string expected("0b");
        expected +=
            "00000000"
            "00000000"
            "00000000"
            "00000000";
        expected +=
            "00000000"
            "00000000"
            "00000000"
            "00000000";
        expected +=
            "01111111"
            "11111111"
            "11111111"
            "11111111";
        expected +=
            "11111111"
            "11111111"
            "11111111"
            "11111111";

        int64_t i64   = std::numeric_limits<int64_t>::max();
        uint64_t u64  = i64;
        int128_t i128 = u64;

        REQUIRE(nonstd::to_string_binary(i128) == expected);
    }

    SECTION("uint64_t(min) to int128_t pads with zero") {
        std::string expected("0b");
        expected +=
            "00000000"
            "00000000"
            "00000000"
            "00000000";
        expected +=
            "00000000"
            "00000000"
            "00000000"
            "00000000";
        expected +=
            "00000000"
            "00000000"
            "00000000"
            "00000000";
        expected +=
            "00000000"
            "00000000"
            "00000000"
            "00000000";

        uint64_t u64  = std::numeric_limits<uint64_t>::min();
        int128_t i128 = u64;

        REQUIRE(nonstd::to_string_binary(i128) == expected);
    }

    SECTION("int64_t(min) to int128_t pads with one") {
        std::string expected("0b");
        expected +=
            "11111111"
            "11111111"
            "11111111"
            "11111111";
        expected +=
            "11111111"
            "11111111"
            "11111111"
            "11111111";
        expected +=
            "10000000"
            "00000000"
            "00000000"
            "00000000";
        expected +=
            "00000000"
            "00000000"
            "00000000"
            "00000000";

        int64_t i64   = std::numeric_limits<int64_t>::min();
        int128_t i128 = i64;

        REQUIRE(nonstd::to_string_binary(i128) == expected);
    }

    SECTION("int64_t(uint64_t(min)) to int128_t pads with zero") {
        std::string expected("0b");
        expected +=
            "00000000"
            "00000000"
            "00000000"
            "00000000";
        expected +=
            "00000000"
            "00000000"
            "00000000"
            "00000000";
        expected +=
            "00000000"
            "00000000"
            "00000000"
            "00000000";
        expected +=
            "00000000"
            "00000000"
            "00000000"
            "00000000";

        uint64_t u64  = std::numeric_limits<uint64_t>::min();
        int64_t i64   = u64;
        int128_t i128 = i64;

        REQUIRE(nonstd::to_string_binary(i128) == expected);
    }

    SECTION("uint64_t(int64_t(min)) to int128_t pads with zero") {
        std::string expected("0b");
        expected +=
            "00000000"
            "00000000"
            "00000000"
            "00000000";
        expected +=
            "00000000"
            "00000000"
            "00000000"
            "00000000";
        expected +=
            "10000000"
            "00000000"
            "00000000"
            "00000000";
        expected +=
            "00000000"
            "00000000"
            "00000000"
            "00000000";

        int64_t i64   = std::numeric_limits<int64_t>::min();
        uint64_t u64  = i64;
        int128_t i128 = u64;

        REQUIRE(nonstd::to_string_binary(i128) == expected);
    }
}