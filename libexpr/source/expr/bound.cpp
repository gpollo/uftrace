#include "libexpr/expr/bound.hpp"
#include "libexpr/context.hpp"

namespace libexpr {

/* upper bound expression */

upper_bound_expr::upper_bound_expr(context& context, expr_ptr child1, expr_ptr child2)
    : two_child_expr(context, child1, child2) {}

upper_bound_expr::~upper_bound_expr() {}

expr_ptr upper_bound_expr::deep_copy() const {
    auto child1 = child1_->deep_copy();
    auto child2 = child2_->deep_copy();
    return context_.make_expr<upper_bound_expr>(child1, child2);
}

std::string upper_bound_expr::get_name() const {
    return "upper-bound";
}

std::optional<int128_t> upper_bound_expr::evaluate(int128_t child1_result, int128_t child2_result) const {
    if (child1_result > child2_result) {
        return {};
    }

    return child1_result;
}

std::optional<range> upper_bound_expr::get_possible_values(const expr* for_child, int128_t other_child_result,
                                                           range parent_values) const {
    if (child2_.get() == for_child) {
        return range();
    }

    /* TODO: fix casting, use int128_t in range */
    range bound_range(range::infinity::value, static_cast<int64_t>(other_child_result));
    return parent_values.intersection(bound_range);
}

} /* namespace libexpr */