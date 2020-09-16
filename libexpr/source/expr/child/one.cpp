#include "libexpr/expr/child/one.hpp"

namespace libexpr {

one_child_expr::one_child_expr(context& context, expr_ptr child) : expr(context), child_(child) {}

one_child_expr::~one_child_expr() {}

void one_child_expr::init_children() {
    child_->set_parent(weak_from_this());
}

expr_ptrs one_child_expr::get_children() const {
    return {child_};
}

bool one_child_expr::replace_child(expr_ptr child, expr_ptr replacement) {
    if (child_ == child) {
        child_ = replacement;
        replacement->set_parent(shared_from_this());
        return true;
    }

    return false;
}

bool one_child_expr::equal_to(const expr_ptr other) const {
    if (typeid(*this) != typeid(*other)) {
        return false;
    }

    auto other_expr = std::dynamic_pointer_cast<one_child_expr>(other);

    return child_->equal_to(other_expr->child_);
}

std::optional<int128_t> one_child_expr::evaluate_impl() const {
    auto child_result = child_->evaluate();
    if (!child_result.has_value()) {
        return {};
    }

    return evaluate_impl(child_result.value());
}

} /* namespace libexpr */