#include "libexpr/expr/operation.hpp"
#include "libexpr/context.hpp"

namespace libexpr {

/* add expression */

add_expr::add_expr(context& context, expr_ptr child1, expr_ptr child2) : two_child_expr(context, child1, child2) {}

add_expr::~add_expr() {}

expr_ptr add_expr::deep_copy() const {
    auto child1 = child1_->deep_copy();
    auto child2 = child2_->deep_copy();
    return context_.make_expr<add_expr>(child1, child2);
}

std::string add_expr::get_name() const {
    return "add";
}

std::optional<int128_t> add_expr::evaluate(int128_t child1_result, int128_t child2_result) const {
    return child1_result + child2_result;
}

std::optional<range> add_expr::get_possible_values(const expr* for_child, int128_t other_child_result,
                                                   range parent_values) const {
    return parent_values.sub(other_child_result);
}

/* multiplication expression */

mul_expr::mul_expr(context& context, expr_ptr child1, expr_ptr child2) : two_child_expr(context, child1, child2) {}

mul_expr::~mul_expr() {}

expr_ptr mul_expr::deep_copy() const {
    auto child1 = child1_->deep_copy();
    auto child2 = child1_->deep_copy();
    return context_.make_expr<mul_expr>(child1, child2);
}

std::string mul_expr::get_name() const {
    return "mul";
}

std::optional<int128_t> mul_expr::evaluate(int128_t child1_result, int128_t child2_result) const {
    return child1_result * child2_result;
}

std::optional<range> mul_expr::get_possible_values(const expr* for_child, int128_t other_child_result,
                                                   range parent_values) const {
    return parent_values.div(other_child_result);
}

} /* namespace libexpr */