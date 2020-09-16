#include "libexpr/expr/extend.hpp"
#include "catch.hpp"
#include "dummy/memory.hpp"
#include "libexpr/expr/constant.hpp"

TEST_CASE("zero 32-bit extension should be valid", "[libexpr::zero_extend32_expr]") {
    tests::dummy_memory memory;
    libexpr::context context(memory);

    SECTION("positive value is extended as unsigned") {
        auto constant      = context.make_expr<libexpr::constant_expr>(1);
        auto result1       = constant->evaluate();
        int128_t expected1 = nonstd::make_u128(0x00000000, 0x00000000, 0x00000000, 0x00000001);

        REQUIRE(result1.has_value());
        REQUIRE(result1.value() == expected1);

        auto extend        = context.make_expr<libexpr::zero_extend32_expr>(constant);
        auto result2       = extend->evaluate();
        int128_t expected2 = nonstd::make_u128(0x00000000, 0x00000000, 0x00000000, 0x00000001);

        REQUIRE(result2.has_value());
        REQUIRE(result2.value() == expected2);
    }

    SECTION("negative value is extended as unsigned") {
        auto constant      = context.make_expr<libexpr::constant_expr>(-1);
        auto result1       = constant->evaluate();
        int128_t expected1 = nonstd::make_u128(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);

        REQUIRE(result1.has_value());
        REQUIRE(result1.value() == expected1);

        auto extend        = context.make_expr<libexpr::zero_extend32_expr>(constant);
        auto result2       = extend->evaluate();
        int128_t expected2 = nonstd::make_u128(0x00000000, 0x00000000, 0x00000000, 0xFFFFFFFF);

        REQUIRE(result2.has_value());
        REQUIRE(result2.value() == expected2);
    }

    SECTION("big positive value is cropped as unsigned") {
        auto constant      = context.make_expr<libexpr::constant_expr>(0x100000000);
        auto result1       = constant->evaluate();
        int128_t expected1 = nonstd::make_u128(0x00000000, 0x00000000, 0x00000001, 0x00000000);

        REQUIRE(result1.has_value());
        REQUIRE(result1.value() == expected1);

        auto extend        = context.make_expr<libexpr::zero_extend32_expr>(constant);
        auto result2       = extend->evaluate();
        int128_t expected2 = nonstd::make_u128(0x00000000, 0x00000000, 0x00000000, 0x00000000);

        REQUIRE(result2.has_value());
        REQUIRE(result2.value() == expected2);
    }

    SECTION("big negative value is cropped as unsigned") {
        auto constant      = context.make_expr<libexpr::constant_expr>(-0x100000000);
        auto result1       = constant->evaluate();
        int128_t expected1 = nonstd::make_u128(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000);

        REQUIRE(result1.has_value());
        REQUIRE(result1.value() == expected1);

        auto extend        = context.make_expr<libexpr::zero_extend32_expr>(constant);
        auto result2       = extend->evaluate();
        int128_t expected2 = nonstd::make_u128(0x00000000, 0x00000000, 0x00000000, 0x00000000);

        REQUIRE(result2.has_value());
        REQUIRE(result2.value() == expected2);
    }
}

TEST_CASE("sign 32-bit extension should be valid", "[libexpr::sign_extend32_expr]") {
    tests::dummy_memory memory;
    libexpr::context context(memory);

    SECTION("positive value is extended as signed") {
        auto constant      = context.make_expr<libexpr::constant_expr>(1);
        auto result1       = constant->evaluate();
        int128_t expected1 = nonstd::make_u128(0x00000000, 0x00000000, 0x00000000, 0x00000001);

        REQUIRE(result1.has_value());
        REQUIRE(result1.value() == expected1);

        auto extend        = context.make_expr<libexpr::sign_extend32_expr>(constant);
        auto result2       = extend->evaluate();
        int128_t expected2 = nonstd::make_u128(0x00000000, 0x00000000, 0x00000000, 0x00000001);

        REQUIRE(result2.has_value());
        REQUIRE(result2.value() == expected2);
    }

    SECTION("negative value is extended as signed") {
        auto constant      = context.make_expr<libexpr::constant_expr>(-1);
        auto result1       = constant->evaluate();
        int128_t expected1 = nonstd::make_u128(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);

        REQUIRE(result1.has_value());
        REQUIRE(result1.value() == expected1);

        auto extend        = context.make_expr<libexpr::sign_extend32_expr>(constant);
        auto result2       = extend->evaluate();
        int128_t expected2 = nonstd::make_u128(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);

        REQUIRE(result2.has_value());
        REQUIRE(result2.value() == expected2);
    }

    SECTION("big positive value is cropped as signed") {
        auto constant      = context.make_expr<libexpr::constant_expr>(0x100000000);
        auto result1       = constant->evaluate();
        int128_t expected1 = nonstd::make_u128(0x00000000, 0x00000000, 0x00000001, 0x00000000);

        REQUIRE(result1.has_value());
        REQUIRE(result1.value() == expected1);

        auto extend        = context.make_expr<libexpr::sign_extend32_expr>(constant);
        auto result2       = extend->evaluate();
        int128_t expected2 = nonstd::make_u128(0x00000000, 0x00000000, 0x00000000, 0x00000000);

        REQUIRE(result2.has_value());
        REQUIRE(result2.value() == expected2);
    }

    SECTION("big negative value is cropped as signed") {
        auto constant      = context.make_expr<libexpr::constant_expr>(-0x100000000);
        auto result1       = constant->evaluate();
        int128_t expected1 = nonstd::make_u128(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000);

        REQUIRE(result1.has_value());
        REQUIRE(result1.value() == expected1);

        auto extend        = context.make_expr<libexpr::sign_extend32_expr>(constant);
        auto result2       = extend->evaluate();
        int128_t expected2 = nonstd::make_u128(0x00000000, 0x00000000, 0x00000000, 0x00000000);

        REQUIRE(result2.has_value());
        REQUIRE(result2.value() == expected2);
    }
}
