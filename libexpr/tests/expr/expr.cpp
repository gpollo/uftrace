#include "catch.hpp"
#include "dummy/memory.hpp"
#include "libexpr/expr/register.hpp"

TEST_CASE("evaluating value from context works", "[libexpr::expr]") {
    tests::dummy_memory memory;
    libexpr::context context(memory);

    SECTION("should return no value if register value is not set") {
        auto register_rax = context.make_expr<libexpr::register_expr>("rax");
        REQUIRE(!register_rax->evaluate().has_value());
    }

    SECTION("should return value if register value is set") {
        auto register_rax = context.make_expr<libexpr::register_expr>("rax");
        context.set_value(register_rax, 100);
        REQUIRE(register_rax->evaluate().has_value());
        REQUIRE(register_rax->evaluate().value() == 100);
    }
}
