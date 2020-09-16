#include "catch.hpp"
#include "dummy/memory.hpp"
#include "libexpr/expr/constant.hpp"
#include "libexpr/expr/operation.hpp"

TEST_CASE("tree equality works", "[libexpr::two_child_expr]") {
    tests::dummy_memory memory;
    libexpr::context context(memory);

    SECTION("tree is equal to itself") {
        auto constant_100 = context.make_expr<libexpr::constant_expr>(100);
        auto constant_300 = context.make_expr<libexpr::constant_expr>(300);
        auto add          = context.make_expr<libexpr::add_expr>(constant_100, constant_300);

        REQUIRE(add->equal_to(add));
    }

    SECTION("tree is equal to an equivalent object") {
        auto constant_100 = context.make_expr<libexpr::constant_expr>(100);
        auto constant_300 = context.make_expr<libexpr::constant_expr>(300);
        auto add1         = context.make_expr<libexpr::add_expr>(constant_100, constant_300);
        auto add2         = context.make_expr<libexpr::add_expr>(constant_100, constant_300);

        REQUIRE(add1->equal_to(add2));
        REQUIRE(add2->equal_to(add1));

        auto constant_100_other = context.make_expr<libexpr::constant_expr>(100);
        auto constant_300_other = context.make_expr<libexpr::constant_expr>(300);
        auto add3               = context.make_expr<libexpr::add_expr>(constant_100_other, constant_300_other);

        REQUIRE(add1->equal_to(add3));
        REQUIRE(add3->equal_to(add1));

        REQUIRE(add2->equal_to(add3));
        REQUIRE(add3->equal_to(add2));
    }

    SECTION("tree is equal to an equivalent object, but swapped") {
        auto constant_100 = context.make_expr<libexpr::constant_expr>(100);
        auto constant_300 = context.make_expr<libexpr::constant_expr>(300);
        auto add1         = context.make_expr<libexpr::add_expr>(constant_100, constant_300);
        auto add2         = context.make_expr<libexpr::add_expr>(constant_300, constant_100);

        REQUIRE(add1->equal_to(add2));
        REQUIRE(add2->equal_to(add1));

        auto constant_100_other = context.make_expr<libexpr::constant_expr>(100);
        auto constant_300_other = context.make_expr<libexpr::constant_expr>(300);
        auto add3               = context.make_expr<libexpr::add_expr>(constant_300_other, constant_100_other);

        REQUIRE(add1->equal_to(add3));
        REQUIRE(add3->equal_to(add1));

        REQUIRE(add2->equal_to(add3));
        REQUIRE(add3->equal_to(add2));
    }

    SECTION("tree is not equal to different root type") {
        auto constant_100 = context.make_expr<libexpr::constant_expr>(100);
        auto constant_300 = context.make_expr<libexpr::constant_expr>(300);
        auto add          = context.make_expr<libexpr::add_expr>(constant_100, constant_300);
        auto mul          = context.make_expr<libexpr::mul_expr>(constant_100, constant_300);

        REQUIRE(!add->equal_to(mul));
        REQUIRE(!mul->equal_to(add));
    }

    SECTION("tree is not equal to different children") {
        auto constant_100 = context.make_expr<libexpr::constant_expr>(100);
        auto constant_200 = context.make_expr<libexpr::constant_expr>(200);
        auto constant_300 = context.make_expr<libexpr::constant_expr>(300);
        auto add1         = context.make_expr<libexpr::add_expr>(constant_100, constant_300);
        auto add2         = context.make_expr<libexpr::add_expr>(constant_200, constant_300);
        auto add3         = context.make_expr<libexpr::add_expr>(constant_100, constant_200);

        REQUIRE(!add1->equal_to(add2));
        REQUIRE(!add1->equal_to(add3));

        REQUIRE(!add2->equal_to(add1));
        REQUIRE(!add2->equal_to(add3));

        REQUIRE(!add3->equal_to(add1));
        REQUIRE(!add3->equal_to(add2));
    }
}
