#ifndef LIBEXPR_INCLUDE_EXPR_BOUND_HPP
#define LIBEXPR_INCLUDE_EXPR_BOUND_HPP

#include "libexpr/expr/child/two.hpp"

namespace libexpr {

/**
 * This expression gives an upper bound, defined by the second
 * child, to the first child.
 */
class upper_bound_expr : public two_child_expr {
   public:
    upper_bound_expr(context& context, expr_ptr child1, expr_ptr child2);
    virtual ~upper_bound_expr();
    virtual expr_ptr deep_copy() const;

    virtual std::string get_name() const;
    virtual std::optional<int128_t> evaluate(int128_t child1_result, int128_t child2_result) const;

   private:
    virtual std::optional<range> get_possible_values(const expr* for_child, int128_t other_child_result,
                                                     range parent_values) const;
};

} /* namespace libexpr */

#endif /* LIBEXPR_INCLUDE_EXPR_BOUND_HPP */