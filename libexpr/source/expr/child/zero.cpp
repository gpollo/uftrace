#include "libexpr/expr/child/zero.hpp"

namespace libexpr {

zero_child_expr::zero_child_expr(context& context) : expr(context) {}

zero_child_expr::~zero_child_expr() {}

void zero_child_expr::init_children() {}

expr_ptrs zero_child_expr::get_children() const {
    return {};
}

bool zero_child_expr::replace_child(expr_ptr child, expr_ptr replacement) {
    return false;
}

std::optional<range> zero_child_expr::get_possible_values(const expr* for_child, range parent_values) const {
    return parent_values;
}

} /* namespace libexpr */