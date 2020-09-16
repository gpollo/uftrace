#include <algorithm>
#include <functional>
#include <iostream>
#include <tuple>

#include "libexpr/range.hpp"

namespace libexpr {

/* nested types */

const range::empty range::empty::value;
const range::infinity range::infinity::value;
const std::unordered_set<range::integer> range::excluded_empty_;

bool range::infinity::operator==(const infinity& other) const {
    return true;
}

bool range::infinity::operator!=(const infinity& other) const {
    return false;
}

range::infinity::infinity() {}

range::empty::empty() {}

static inline bool is_infinity(const range::value& v) {
    return std::holds_alternative<range::infinity>(v);
}

static inline range::integer get_value(const range::value& v) {
    return std::get<range::integer>(v);
}

/* range members */

range::range() {}

range::range(empty empty) : empty_(true) {}

range::range(std::unordered_set<range::integer> excluded) : excluded_(excluded) {}

range::range(std::initializer_list<range::integer> excluded) : excluded_(excluded) {}

range::range(range::value minimum, range::value maximum) : minimum_(minimum), maximum_(maximum) {
    init(minimum, maximum);
}

range::range(range::value minimum, range::value maximum, std::unordered_set<range::integer> excluded)
    : minimum_(minimum), maximum_(maximum), excluded_(excluded) {
    init(minimum, maximum);
}

range::range(range::value minimum, range::value maximum, std::initializer_list<range::integer> excluded)
    : minimum_(minimum), maximum_(maximum), excluded_(excluded) {
    init(minimum, maximum);
}

void range::init(range::value minimum, range::value maximum) {
    if (!is_infinity(minimum) && !is_infinity(maximum)) {
        if (get_value(minimum) > get_value(maximum)) {
            empty_   = true;
            minimum_ = range::infinity::value;
            maximum_ = range::infinity::value;
        }
    }
}

bool range::is_empty() const {
    return empty_;
}

bool range::is_unbounded_negative() const {
    return is_empty() ? false : is_infinity(minimum_);
}

bool range::is_unbounded_positive() const {
    return is_empty() ? false : is_infinity(maximum_);
}

bool range::is_unbounded() const {
    return is_empty() ? false : (is_unbounded_negative() && is_unbounded_positive());
}

std::optional<range::value> range::get_minimum() const {
    if (is_empty()) {
        return {};
    } else {
        return minimum_;
    }
}

std::optional<range::value> range::get_maximum() const {
    if (is_empty()) {
        return {};
    } else {
        return maximum_;
    }
}

std::optional<range::integer> range::get_finite_minimum() const {
    if (is_empty() || is_unbounded_negative()) {
        return {};
    } else {
        return std::get<range::integer>(minimum_);
    }
}

std::optional<range::integer> range::get_finite_maximum() const {
    if (is_empty() || is_unbounded_positive()) {
        return {};
    } else {
        return std::get<range::integer>(maximum_);
    }
}

const std::unordered_set<range::integer>& range::get_excluded() const {
    if (is_empty()) {
        return excluded_empty_;
    } else {
        return excluded_;
    }
}

std::unordered_set<range::integer> range::contained_of(std::unordered_set<range::integer>& values) {
    if (is_empty()) {
        return {};
    }

    if (is_unbounded()) {
        return values;
    }

    std::unordered_set<range::integer> results;
    auto inserter = std::inserter(results, results.end());

    std::copy_if(values.begin(), values.end(), inserter, get_in_range_predicate());
    return results;
}

std::pair<range::value, range::value> intersects(const range::value& min1, const range::value& max1,
                                                 const range::value& min2, const range::value& max2) {
    range::value new_minimum;
    if (is_infinity(min1)) {
        if (is_infinity(min2)) {
            new_minimum = range::infinity::value;
        } else {
            new_minimum = min2;
        }
    } else {
        if (is_infinity(min2)) {
            new_minimum = min1;
        } else {
            new_minimum = (std::get<range::integer>(min1) < std::get<range::integer>(min2)) ? min2 : min1;
        }
    }

    range::value new_maximum;
    if (is_infinity(max1)) {
        if (is_infinity(max2)) {
            new_maximum = range::infinity::value;
        } else {
            new_maximum = max2;
        }
    } else {
        if (is_infinity(max2)) {
            new_maximum = max1;
        } else {
            new_maximum = (std::get<range::integer>(max1) < std::get<range::integer>(max2)) ? max1 : max2;
        }
    }

    return {new_minimum, new_maximum};
}

bool range::intersects(range& other) const {
    if (is_empty() || other.is_empty()) {
        return false;
    }

    auto min_max = libexpr::intersects(minimum_, maximum_, other.minimum_, other.maximum_);

    return !range(min_max.first, min_max.second).is_empty();
}

range range::intersection(range& other) const {
    if (is_empty() || other.is_empty()) {
        return range(range::empty::value);
    }

    auto min_max = libexpr::intersects(minimum_, maximum_, other.minimum_, other.maximum_);

    range new_range(min_max.first, min_max.second);
    if (new_range.is_empty()) {
        return range(range::empty::value);
    }

    auto inserter = std::inserter(new_range.excluded_, new_range.excluded_.end());
    auto in_range = new_range.get_in_range_predicate();

    std::copy_if(excluded_.begin(), excluded_.end(), inserter, in_range);
    std::copy_if(other.excluded_.begin(), other.excluded_.end(), inserter, in_range);

    return new_range;
}

range range::add(range::integer value) const {
    if (is_empty()) {
        return range(range::empty::value);
    }

    range::value new_minimum;
    if (is_unbounded_negative()) {
        new_minimum = minimum_;
    } else {
        new_minimum = get_value(minimum_) + value;
    }

    range::value new_maximum;
    if (is_unbounded_positive()) {
        new_maximum = maximum_;
    } else {
        new_maximum = get_value(maximum_) + value;
    }

    std::unordered_set<range::integer> new_excluded;
    for (auto excluded : excluded_) {
        new_excluded.insert(excluded + value);
    }

    return range(new_minimum, new_maximum, new_excluded);
}

range range::sub(range::integer value) const {
    return add(-value);
}

range range::div(range::integer value) const {
    if (value == 0) {
        return range();
    }

    range::value new_minimum;
    if (is_unbounded_negative()) {
        new_minimum = minimum_;
    } else {
        new_minimum = get_value(minimum_) / value;
    }

    range::value new_maximum;
    if (is_unbounded_positive()) {
        new_maximum = maximum_;
    } else {
        new_maximum = get_value(maximum_) / value;
    }

    if (value < 0) {
        std::swap(new_minimum, new_maximum);
    }

    std::unordered_set<range::integer> new_excluded;
    for (auto excluded : excluded_) {
        new_excluded.insert(excluded / value);
    }

    return range(new_minimum, new_maximum, new_excluded);
}

range::in_range range::get_in_range_predicate() const {
    return in_range(*this);
}

range::in_range::in_range(const range& range) : range_(range) {}

bool range::in_range::operator()(range::integer value) const {
    if (range_.is_empty()) {
        return false;
    }

    if (range_.is_unbounded()) {
        return true;
    }

    if (range_.is_unbounded_negative()) {
        return value <= range_.get_finite_maximum().value();
    }

    if (range_.is_unbounded_positive()) {
        return range_.get_finite_minimum().value() <= value;
    }

    auto minimum = range_.get_finite_minimum().value();
    auto maximum = range_.get_finite_maximum().value();

    return (minimum <= value) && (value <= maximum);
}

bool range::operator==(const range& other) const {
    if (is_empty() && other.is_empty()) {
        return true;
    }

    if (is_unbounded() && other.is_unbounded()) {
        return (excluded_ == other.excluded_);
    }

    if (is_unbounded_negative() && other.is_unbounded_negative()) {
        return (excluded_ == other.excluded_) && (maximum_ == other.maximum_);
    }

    if (is_unbounded_positive() && other.is_unbounded_positive()) {
        return (excluded_ == other.excluded_) && (minimum_ == other.minimum_);
    }

    if (excluded_ != other.excluded_) {
        return false;
    }

    return (minimum_ == other.minimum_) && (maximum_ == other.maximum_);
}

std::ostream& operator<<(std::ostream& os, const range& range) {
    if (range.is_unbounded()) {
        os << "]-oo, +oo[";
    } else if (range.is_unbounded_negative()) {
        os << "]-oo, " << range.get_finite_maximum().value() << "]";
    } else if (range.is_unbounded_positive()) {
        os << "[" << range.get_finite_minimum().value() << ", +oo[";
    } else {
        os << "[" << range.get_finite_minimum().value() << ", " << range.get_finite_maximum().value() << "]";
    }

    return os;
}

} /* namespace libexpr */