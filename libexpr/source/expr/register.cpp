#include "libexpr/expr/register.hpp"
#include "libexpr/context.hpp"

namespace libexpr {

register_expr::register_expr(context& context, std::string name) : zero_child_expr(context), name_(name) {}

register_expr::~register_expr() {}

expr_ptr register_expr::deep_copy() const {
    return context_.make_expr<register_expr>(name_);
}

std::string register_expr::get_name() const {
    return "register-" + name_;
}

std::optional<int128_t> register_expr::evaluate_impl() const {
    return context_.get_value(shared_from_this());
}

bool register_expr::equal_to(const expr_ptr other) const {
    if (typeid(*this) != typeid(*other)) {
        return false;
    }

    auto other_expr = std::dynamic_pointer_cast<register_expr>(other);

    return (name_ == other_expr->name_);
}

std::string register_expr::get_register_name() const {
    return name_;
}

} /* namespace libexpr */