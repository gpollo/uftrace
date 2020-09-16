#ifndef LIBEXPR_INCLUDE_EXPR_REGISTER_HPP
#define LIBEXPR_INCLUDE_EXPR_REGISTER_HPP

#include "libexpr/expr/child/zero.hpp"

namespace libexpr {

class register_expr : public zero_child_expr {
   public:
    register_expr(context& context, std::string name);
    virtual ~register_expr();
    virtual expr_ptr deep_copy() const;

    virtual std::string get_name() const;
    virtual std::optional<int128_t> evaluate_impl() const;
    virtual bool equal_to(const expr_ptr other) const;

    std::string get_register_name() const;

   private:
    std::string name_;
};

} /* namespace libexpr */

#endif /* LIBEXPR_INCLUDE_EXPR_REGISTER_HPP */