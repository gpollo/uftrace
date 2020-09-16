#include "libexpr/range.hpp"
#include "catch.hpp"

static bool intersections_equal(libexpr::range r1, libexpr::range r2, libexpr::range::value min,
                                libexpr::range::value max,
                                std::initializer_list<libexpr::range::integer> excluded_values) {
    std::unordered_set<libexpr::range::integer> excluded(excluded_values);

    auto r3 = r1.intersection(r2);
    auto r4 = r2.intersection(r1);

    if (r1.is_empty() || r2.is_empty()) {
        if (!r3.is_empty() || !r4.is_empty()) {
            return false;
        }
    }

    auto r3_min = r3.get_minimum().value();
    auto r3_max = r3.get_maximum().value();
    if (r3_min != min || r3_max != max) {
        return false;
    }

    auto r4_min = r4.get_minimum().value();
    auto r4_max = r4.get_maximum().value();
    if (r4_min != min || r4_max != max) {
        return false;
    }

    if (r3.get_excluded() != excluded) {
        return false;
    }

    if (r4.get_excluded() != excluded) {
        return false;
    }

    return true;
}

TEST_CASE("range intersection works", "[libexpr::range]") {
    SECTION("[-oo, +oo] intersection [-oo, +oo] = [-oo, +oo]") {
        libexpr::range r1(libexpr::range::infinity::value, libexpr::range::infinity::value);
        libexpr::range r2(libexpr::range::infinity::value, libexpr::range::infinity::value);
        REQUIRE(intersections_equal(r1, r2, libexpr::range::infinity::value, libexpr::range::infinity::value, {}));
    }

    SECTION("[-5, +oo] intersection [-oo, +oo] = [-5, +oo]") {
        libexpr::range r1(-5, libexpr::range::infinity::value);
        libexpr::range r2(libexpr::range::infinity::value, libexpr::range::infinity::value);
        REQUIRE(intersections_equal(r1, r2, -5, libexpr::range::infinity::value, {}));
    }

    SECTION("[-5, +oo] intersection [0, +oo] = [0, +oo]") {
        libexpr::range r1(-5, libexpr::range::infinity::value);
        libexpr::range r2(0, libexpr::range::infinity::value);
        REQUIRE(intersections_equal(r1, r2, 0, libexpr::range::infinity::value, {}));
    }

    SECTION("[-oo, 5] intersection [-oo, +oo] = [-oo, 5]") {
        libexpr::range r1(libexpr::range::infinity::value, 5);
        libexpr::range r2(libexpr::range::infinity::value, libexpr::range::infinity::value);
        REQUIRE(intersections_equal(r1, r2, libexpr::range::infinity::value, 5, {}));
    }

    SECTION("[-oo, 5] intersection [-oo, 0] = [-oo, 0]") {
        libexpr::range r1(libexpr::range::infinity::value, 5);
        libexpr::range r2(libexpr::range::infinity::value, 0);
        REQUIRE(intersections_equal(r1, r2, libexpr::range::infinity::value, 0, {}));
    }

    SECTION("[-5, 5] intersection [-2, 2] = [-2, 2]") {
        libexpr::range r1(-5, 5);
        libexpr::range r2(-2, 2);
        REQUIRE(intersections_equal(r1, r2, -2, 2, {}));
    }

    SECTION("[-2, 5] intersection [-5, 2] = [-2, 2]") {
        libexpr::range r1(-2, 5);
        libexpr::range r2(-5, 2);
        REQUIRE(intersections_equal(r1, r2, -2, 2, {}));
    }

    SECTION("[-2, 2] intersection [-2, 2] = [-2, 2]") {
        libexpr::range r1(-2, 2);
        libexpr::range r2(-2, 2);
        REQUIRE(intersections_equal(r1, r2, -2, 2, {}));
    }
}

TEST_CASE("range exclusion works", "[libexpr::range]") {
    SECTION("[-oo, +oo] / {1, 2} intersection [-oo, +oo] / {4, 5} = [-oo, +oo] / {1, 2, 4, 5}") {
        libexpr::range r1({1, 2});
        libexpr::range r2({4, 5});
        REQUIRE(intersections_equal(r1, r2, libexpr::range::infinity::value, libexpr::range::infinity::value,
                                    {1, 2, 4, 5}));
    }

    SECTION("[2, +oo] / {1, 2} intersection [-oo, +oo] / {4, 5} = [2, +oo] / {2, 4, 5}") {
        libexpr::range r1(2, libexpr::range::infinity::value, {1, 2});
        libexpr::range r2(libexpr::range::infinity::value, libexpr::range::infinity::value, {4, 5});
        REQUIRE(intersections_equal(r1, r2, 2, libexpr::range::infinity::value, {2, 4, 5}));
    }

    SECTION("[3, +oo] / {1, 3, 5, 7, 9} intersection [-oo, 8] / {0, 2, 4, 6, 8} = [3, 8] / {3, 4, 5, 6, 7, 8}") {
        libexpr::range r1(3, libexpr::range::infinity::value, {1, 3, 5, 7, 9});
        libexpr::range r2(libexpr::range::infinity::value, 8, {0, 2, 4, 6, 8});
        REQUIRE(intersections_equal(r1, r2, 3, 8, {3, 4, 5, 6, 7, 8}));
    }
}

TEST_CASE("contained values works", "[libexpr::range]") {
    SECTION("which of {-5, 0, 5} is in [-oo, +oo] = {-5, 0, 5}") {
        libexpr::range r;
        auto data     = std::unordered_set<libexpr::range::integer>({-5, 0, 5});
        auto expected = std::unordered_set<libexpr::range::integer>({-5, 0, 5});
        auto results  = r.contained_of(data);
        REQUIRE(expected == results);
    }

    SECTION("which of {-5, 0, 5} is in [0, +oo] = {0, 5}") {
        libexpr::range r(0, libexpr::range::infinity::value);
        auto data     = std::unordered_set<libexpr::range::integer>({-5, 0, 5});
        auto expected = std::unordered_set<libexpr::range::integer>({0, 5});
        auto results  = r.contained_of(data);
        REQUIRE(expected == results);
    }

    SECTION("which of {-5, 0, 5} is in [-oo, 0] = {-5, 0}") {
        libexpr::range r(libexpr::range::infinity::value, 0);
        auto data     = std::unordered_set<libexpr::range::integer>({-5, 0, 5});
        auto expected = std::unordered_set<libexpr::range::integer>({-5, 0});
        auto results  = r.contained_of(data);
        REQUIRE(expected == results);
    }

    SECTION("which of {-5, 0, 5} is in [-1, 1] = {0}") {
        libexpr::range r(-1, 1);
        auto data     = std::unordered_set<libexpr::range::integer>({-5, 0, 5});
        auto expected = std::unordered_set<libexpr::range::integer>({0});
        auto results  = r.contained_of(data);
        REQUIRE(expected == results);
    }
}

TEST_CASE("addition works", "[libexpr::range]") {
    SECTION("∅ + 5 = ∅") {
        libexpr::range r1(libexpr::range::empty::value);
        libexpr::range r2 = r1.add(5);
        REQUIRE(r2.is_empty());
        REQUIRE(r2.get_excluded() == std::unordered_set<libexpr::range::integer>());
        REQUIRE(!r2.get_finite_minimum().has_value());
        REQUIRE(!r2.get_finite_maximum().has_value());
    }

    SECTION("[-oo, +oo] + 5 = [-oo, +oo]") {
        libexpr::range r1;
        libexpr::range r2 = r1.add(5);
        REQUIRE(!r2.is_empty());
        REQUIRE(std::get<libexpr::range::infinity>(r2.get_minimum().value()) == libexpr::range::infinity::value);
        REQUIRE(std::get<libexpr::range::infinity>(r2.get_maximum().value()) == libexpr::range::infinity::value);
        REQUIRE(!r2.get_finite_minimum().has_value());
        REQUIRE(!r2.get_finite_maximum().has_value());
        REQUIRE(r2.get_excluded() == std::unordered_set<libexpr::range::integer>());
    }

    SECTION("[-10, +oo] + 5 = [-5, +oo]") {
        libexpr::range r1(-10, libexpr::range::infinity::value);
        libexpr::range r2 = r1.add(5);
        REQUIRE(!r2.is_empty());
        REQUIRE(std::get<libexpr::range::integer>(r2.get_minimum().value()) == -5);
        REQUIRE(std::get<libexpr::range::infinity>(r2.get_maximum().value()) == libexpr::range::infinity::value);
        REQUIRE(r2.get_finite_minimum().value() == -5);
        REQUIRE(!r2.get_finite_maximum().has_value());
        REQUIRE(r2.get_excluded() == std::unordered_set<libexpr::range::integer>());
    }

    SECTION("[-10, 10] - 5 = [-15, 5]") {
        libexpr::range r1(-10, 10);
        libexpr::range r2 = r1.add(-5);
        REQUIRE(!r2.is_empty());
        REQUIRE(std::get<libexpr::range::integer>(r2.get_minimum().value()) == -15);
        REQUIRE(std::get<libexpr::range::integer>(r2.get_maximum().value()) == 5);
        REQUIRE(r2.get_finite_minimum().value() == -15);
        REQUIRE(r2.get_finite_maximum().value() == 5);
        REQUIRE(r2.get_excluded() == std::unordered_set<libexpr::range::integer>());
    }

    SECTION("([-10, 10] / {0, 1, 2, 3, 4, 5}) - 5 = ([-15, 5] / {-5, -4, -3, -2, -1, 0})") {
        libexpr::range r1(-10, 10, {0, 1, 2, 3, 4, 5});
        libexpr::range r2 = r1.add(-5);
        REQUIRE(!r2.is_empty());
        REQUIRE(std::get<libexpr::range::integer>(r2.get_minimum().value()) == -15);
        REQUIRE(std::get<libexpr::range::integer>(r2.get_maximum().value()) == 5);
        REQUIRE(r2.get_finite_minimum().value() == -15);
        REQUIRE(r2.get_finite_maximum().value() == 5);
        REQUIRE(r2.get_excluded() == std::unordered_set<libexpr::range::integer>({-5, -4, -3, -2, -1, 0}));
    }
}