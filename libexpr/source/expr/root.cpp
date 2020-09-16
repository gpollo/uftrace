#include "libexpr/expr/root.hpp"
#include "libexpr/context.hpp"

namespace libexpr {

root_expr::root_expr(context& context, expr_ptr child) : one_child_expr(context, child) {}

root_expr::~root_expr() {}

expr_ptr root_expr::deep_copy() const {
    /* TODO: that won't work */
    auto child = child_->deep_copy();
    return context_.make_expr<root_expr>(child);
}

std::string root_expr::get_name() const {
    return "root";
}

std::optional<int128_t> root_expr::evaluate_impl(int128_t child_result) const {
    return child_result;
}

std::optional<range> root_expr::get_possible_values(const expr* for_child, range parent_values) const {
    return range();
}

} /* namespace libexpr */