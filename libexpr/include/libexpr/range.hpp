#ifndef LIBEXPR_RANGE_HPP
#define LIBEXPR_RANGE_HPP

#include <cstdint>
#include <initializer_list>
#include <iostream>
#include <optional>
#include <unordered_set>
#include <variant>

#include "libexpr/utils.hpp"

namespace libexpr {

class range {
   public:
    class infinity;
    class infinity {
       public:
        static const infinity value;
        bool operator==(const infinity& other) const;
        bool operator!=(const infinity& other) const;

       private:
        infinity();
    };

    class empty;
    class empty {
       public:
        static const empty value;

       private:
        empty();
    };

    using integer = int64_t;
    using value   = std::variant<integer, infinity>;

    range();
    range(empty empty);
    range(std::unordered_set<integer> excluded);
    range(std::initializer_list<integer> excluded);
    range(value minimum, value maximum);
    range(value minimum, value maximum, std::unordered_set<integer> excluded);
    range(value minimum, value maximum, std::initializer_list<integer> excluded);

    bool is_empty() const;
    bool is_unbounded_negative() const;
    bool is_unbounded_positive() const;
    bool is_unbounded() const;

    std::optional<value> get_minimum() const;
    std::optional<value> get_maximum() const;
    std::optional<integer> get_finite_minimum() const;
    std::optional<integer> get_finite_maximum() const;

    const std::unordered_set<integer>& get_excluded() const;
    std::unordered_set<integer> contained_of(std::unordered_set<integer>& values);

    bool intersects(range& other) const;
    range intersection(range& other) const;

    range add(integer value) const;
    range sub(integer value) const;
    range div(integer value) const;

    class in_range {
       public:
        in_range(const range& range);
        bool operator()(integer value) const;

       private:
        const range& range_;
    };

    in_range get_in_range_predicate() const;

    bool operator==(const range& other) const;
    friend std::ostream& operator<<(std::ostream& os, const range& range);

   private:
    bool empty_    = false;
    value minimum_ = infinity::value;
    value maximum_ = infinity::value;
    std::unordered_set<integer> excluded_;

    static const std::unordered_set<integer> excluded_empty_;

    void init(range::value minimum, range::value maximum);
};

} /* namespace libexpr */

#endif /* LIBEXPR_RANGE_HPP */
