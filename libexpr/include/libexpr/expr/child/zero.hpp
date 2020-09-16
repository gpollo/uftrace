#ifndef LIBEXPR_INCLUDE_EXPR_CHILD_ZERO_HPP
#define LIBEXPR_INCLUDE_EXPR_CHILD_ZERO_HPP

#include "libexpr/expr/expr.hpp"

namespace libexpr {

class zero_child_expr : public expr {
   public:
    zero_child_expr(context& context);
    virtual ~zero_child_expr();
    virtual void init_children() final;
    virtual expr_ptr deep_copy() const = 0;

    virtual std::string get_name() const = 0;
    virtual expr_ptrs get_children() const final;
    virtual bool replace_child(expr_ptr child, expr_ptr replacement) final;
    virtual bool equal_to(const expr_ptr other) const     = 0;
    virtual std::optional<int128_t> evaluate_impl() const = 0;

   private:
    virtual std::optional<range> get_possible_values(const expr* for_child, range parent_values) const;
};

} /* namespace libexpr */

#endif /* LIBEXPR_INCLUDE_EXPR_CHILD_ZERO_HPP */