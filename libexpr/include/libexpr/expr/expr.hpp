#ifndef LIBEXPR_INCLUDE_EXPR_EXPR_HPP
#define LIBEXPR_INCLUDE_EXPR_EXPR_HPP

#include <iostream>
#include <memory>
#include <optional>
#include <string>

#include "libexpr/range.hpp"
#include "libexpr/utils.hpp"

namespace libexpr {

class context;
class expr;

using expr_ptr       = std::shared_ptr<expr>;
using expr_ptrs      = std::vector<expr_ptr>;
using const_expr_ptr = std::shared_ptr<const expr>;

class expr : public std::enable_shared_from_this<expr> {
   public:
    expr(context& context);
    virtual ~expr();
    virtual void init_children()       = 0;
    virtual expr_ptr deep_copy() const = 0;

    virtual std::string get_name() const                             = 0;
    virtual expr_ptrs get_children() const                           = 0;
    virtual bool replace_child(expr_ptr child, expr_ptr replacement) = 0;
    virtual bool equal_to(const expr_ptr other) const                = 0;
    virtual std::optional<int128_t> evaluate_impl() const            = 0;

    expr_ptr get_parent() const;
    void set_parent(std::weak_ptr<expr> expr);

    std::optional<range> get_possible_values() const;
    std::optional<range> get_possible_values(const expr* for_child) const;
    std::optional<int128_t> evaluate() const;

    friend std::ostream& operator<<(std::ostream& os, const expr_ptr& expr);

   protected:
    context& context_;
    std::weak_ptr<expr> parent_;

    virtual std::optional<range> get_possible_values(const expr* for_child, range parent_values) const = 0;
};

} /* namespace libexpr */

#endif /* LIBEXPR_INCLUDE_EXPR_EXPR_HPP */