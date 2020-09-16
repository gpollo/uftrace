#ifndef LIBEXPR_INCLUDE_EXPR_CHILD_TWO_HPP
#define LIBEXPR_INCLUDE_EXPR_CHILD_TWO_HPP

#include "libexpr/expr/expr.hpp"

namespace libexpr {

class two_child_expr : public expr {
   public:
    two_child_expr(context& context, expr_ptr child1, expr_ptr child2);
    virtual ~two_child_expr();
    virtual void init_children() final;
    virtual expr_ptr deep_copy() const = 0;

    virtual std::string get_name() const = 0;
    virtual expr_ptrs get_children() const final;
    virtual bool replace_child(expr_ptr child, expr_ptr replacement) final;
    virtual bool equal_to(const expr_ptr other) const final;
    virtual std::optional<int128_t> evaluate_impl() const final;
    virtual std::optional<int128_t> evaluate(int128_t child1_result, int128_t child2_result) const = 0;

   protected:
    expr_ptr child1_;
    expr_ptr child2_;

    virtual std::optional<range> get_possible_values(const expr* for_child, int128_t other_child_result,
                                                     range parent_values) const = 0;

   private:
    virtual std::optional<range> get_possible_values(const expr* for_child, range parent_values) const;
};

} /* namespace libexpr */

#endif /* LIBEXPR_INCLUDE_EXPR_CHILD_TWO_HPP */