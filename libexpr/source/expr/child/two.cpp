#include "libexpr/expr/child/two.hpp"

namespace libexpr {

two_child_expr::two_child_expr(context& context, expr_ptr child1, expr_ptr child2)
    : expr(context), child1_(child1), child2_(child2) {}

two_child_expr::~two_child_expr() {}

void two_child_expr::init_children() {
    child1_->set_parent(weak_from_this());
    child2_->set_parent(weak_from_this());
}

expr_ptrs two_child_expr::get_children() const {
    return {child1_, child2_};
}

bool two_child_expr::replace_child(expr_ptr child, expr_ptr replacement) {
    if (child1_ == child) {
        child1_ = replacement;
        replacement->set_parent(shared_from_this());
        return true;
    }

    if (child2_ == child) {
        child2_ = replacement;
        replacement->set_parent(shared_from_this());
        return true;
    }

    return false;
}

bool two_child_expr::equal_to(const expr_ptr other) const {
    if (typeid(*this) != typeid(*other)) {
        return false;
    }

    auto other_expr = std::dynamic_pointer_cast<two_child_expr>(other);

    if (child1_->equal_to(other_expr->child1_)) {
        return child2_->equal_to(other_expr->child2_);
    }

    if (child1_->equal_to(other_expr->child2_)) {
        return child2_->equal_to(other_expr->child1_);
    }

    return false;
}

std::optional<int128_t> two_child_expr::evaluate_impl() const {
    auto child1_result = child1_->evaluate();
    if (!child1_result.has_value()) {
        return {};
    }

    auto child2_result = child2_->evaluate();
    if (!child2_result.has_value()) {
        return {};
    }

    return evaluate(child1_result.value(), child2_result.value());
}

std::optional<range> two_child_expr::get_possible_values(const expr* for_child, range parent_values) const {
    std::optional<int128_t> other_child_result;
    if (child1_.get() == for_child) {
        other_child_result = child2_->evaluate();
    } else {
        other_child_result = child1_->evaluate();
    }

    if (!other_child_result.has_value()) {
        return {};
    }

    return get_possible_values(for_child, other_child_result.value(), parent_values);
}

} /* namespace libexpr */