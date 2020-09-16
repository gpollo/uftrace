#ifndef LIBEXPR_INCLUDE_EXPR_CHILD_ONE_HPP
#define LIBEXPR_INCLUDE_EXPR_CHILD_ONE_HPP

#include "libexpr/expr/expr.hpp"

namespace libexpr {

class one_child_expr : public expr {
   public:
    one_child_expr(context& context, expr_ptr child);
    virtual ~one_child_expr();
    virtual void init_children() final;
    virtual expr_ptr deep_copy() const = 0;

    virtual std::string get_name() const = 0;
    virtual expr_ptrs get_children() const final;
    virtual bool replace_child(expr_ptr child, expr_ptr replacement) final;
    virtual bool equal_to(const expr_ptr other) const final;
    virtual std::optional<int128_t> evaluate_impl() const final;
    virtual std::optional<int128_t> evaluate_impl(int128_t child_result) const = 0;

   protected:
    expr_ptr child_;

    virtual std::optional<range> get_possible_values(const expr* for_child, range parent_values) const = 0;
};

} /* namespace libexpr */

#endif /* LIBEXPR_INCLUDE_EXPR_CHILD_ONE_HPP */