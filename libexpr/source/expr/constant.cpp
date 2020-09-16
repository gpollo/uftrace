#include "libexpr/expr/constant.hpp"
#include "libexpr/context.hpp"

namespace libexpr {

constant_expr::constant_expr(context& context, int128_t value) : zero_child_expr(context), value_(value) {}

constant_expr::~constant_expr() {}

expr_ptr constant_expr::deep_copy() const {
    return context_.make_expr<constant_expr>(value_);
}

std::string constant_expr::get_name() const {
    return "constant-" + nonstd::to_string(value_);
}

std::optional<int128_t> constant_expr::evaluate_impl() const {
    return value_;
}

bool constant_expr::equal_to(const expr_ptr other) const {
    if (typeid(*this) != typeid(*other)) {
        return false;
    }

    auto other_expr = std::dynamic_pointer_cast<constant_expr>(other);

    return (value_ == other_expr->value_);
}

} /* namespace libexpr */