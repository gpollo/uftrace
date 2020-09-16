#ifndef LIBEXPR_INCLUDE_EXPR_ROOT_HPP
#define LIBEXPR_INCLUDE_EXPR_ROOT_HPP

#include "libexpr/expr/child/one.hpp"

namespace libexpr {

class root_expr : public one_child_expr {
   public:
    root_expr(context& context, expr_ptr child);
    virtual ~root_expr();
    virtual expr_ptr deep_copy() const;

    virtual std::string get_name() const final;
    virtual std::optional<int128_t> evaluate_impl(int128_t child_result) const final;

   private:
    virtual std::optional<range> get_possible_values(const expr* for_child, range parent_values) const;
};

} /* namespace libexpr */

#endif /* LIBEXPR_INCLUDE_EXPR_ROOT_HPP */