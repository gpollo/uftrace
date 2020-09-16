#ifndef LIBEXPR_INCLUDE_EXPR_CONSTANT_HPP
#define LIBEXPR_INCLUDE_EXPR_CONSTANT_HPP

#include "libexpr/expr/child/zero.hpp"

namespace libexpr {

class constant_expr : public zero_child_expr {
   public:
    constant_expr(context& context, int128_t value);
    virtual ~constant_expr();
    virtual expr_ptr deep_copy() const;

    virtual std::string get_name() const;
    virtual std::optional<int128_t> evaluate_impl() const;
    virtual bool equal_to(const expr_ptr other) const;

   private:
    int128_t value_;
};

} /* namespace libexpr */

#endif /* LIBEXPR_INCLUDE_EXPR_CONSTANT_HPP */