#include <algorithm>
#include <cassert>
#include <csignal>
#include <stack>

#include "libexpr/context.hpp"
#include "libexpr/expr/expr.hpp"

namespace libexpr {

expr::expr(context& context) : context_(context) {}

expr::~expr() {}

expr_ptr expr::get_parent() const {
    return parent_.lock();
}

void expr::set_parent(std::weak_ptr<expr> expr) {
    parent_ = expr;
}

std::optional<range> expr::get_possible_values() const {
    expr_ptr parent = get_parent();
    if (parent == nullptr) {
        return range();
    }

    return parent->get_possible_values(this);
}

std::optional<range> expr::get_possible_values(const expr* for_child) const {
    expr_ptr parent = get_parent();
    if (parent == nullptr) {
        return range();
    }

    std::optional<range> parent_values = parent->get_possible_values(this);
    if (!parent_values.has_value()) {
        return {};
    }

    auto children = get_children();
    if (children.size() == 0) {
        return parent_values.value();
    }

    for (auto child : children) {
        if (child.get() == for_child) {
            return get_possible_values(for_child, parent_values.value());
        }
    }

    std::cerr << "expr is not a children of this node" << std::endl;
    return {};
}

std::optional<int128_t> expr::evaluate() const {
    auto context_value_opt = context_.get_value(shared_from_this());
    if (context_value_opt.has_value()) {
        return context_value_opt;
    }

    return evaluate_impl();
}

std::ostream& operator<<(std::ostream& os, const expr_ptr& expr) {
    std::stack<expr_ptr> stack;
    unsigned int level = 0;

    stack.push(expr);

    while (!stack.empty()) {
        auto next_expr = stack.top();
        stack.pop();

        if (next_expr == nullptr) {
            level--;
            continue;
        }

        for (unsigned int i = 0; i < level; i++) {
            os << "  ";
        }
        os << next_expr->get_name();

        stack.push(nullptr);
        level++;

        auto children = next_expr->get_children();
        for (auto it = children.rbegin(); it != children.rend(); it++) {
            stack.push(*it);
        }

        if (!stack.empty()) {
            os << std::endl;
        }
    }

    return os;
}

} /* namespace libexpr */