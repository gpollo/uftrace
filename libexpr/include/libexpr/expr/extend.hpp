#ifndef LIBEXPR_INCLUDE_EXPR_EXTEND_HPP
#define LIBEXPR_INCLUDE_EXPR_EXTEND_HPP

#include <limits>

#include "libexpr/context.hpp"
#include "libexpr/expr/child/one.hpp"

namespace libexpr {

template <bool IsSigned, typename FromType>
class extend_expr : public one_child_expr {
   public:
    extend_expr(context& context, expr_ptr child);
    virtual ~extend_expr();
    virtual expr_ptr deep_copy() const;

    virtual std::string get_name() const;
    virtual std::optional<int128_t> evaluate_impl(int128_t child_result) const;

   private:
    virtual std::optional<range> get_possible_values(const expr* for_child, range parent_values) const;
};

template <bool IsSigned, typename FromType>
extend_expr<IsSigned, FromType>::extend_expr(context& context, expr_ptr child) : one_child_expr(context, child) {}

template <bool IsSigned, typename FromType>
extend_expr<IsSigned, FromType>::~extend_expr() {}

template <bool IsSigned, typename FromType>
expr_ptr extend_expr<IsSigned, FromType>::deep_copy() const {
    auto child = child_->deep_copy();
    return context_.make_expr<extend_expr<IsSigned, FromType>>(child);
}

template <bool IsSigned, typename FromType>
std::string extend_expr<IsSigned, FromType>::get_name() const {
    return std::string("extend-") + typeid(FromType).name();
}

template <bool IsSigned, typename FromType>
std::optional<int128_t> extend_expr<IsSigned, FromType>::evaluate_impl(int128_t child_result) const {
    if (IsSigned) {
        return static_cast<FromType>(static_cast<int64_t>(child_result));
    } else {
        return static_cast<FromType>(static_cast<uint64_t>(child_result));
    }
}

template <bool IsSigned, typename FromType>
std::optional<range> extend_expr<IsSigned, FromType>::get_possible_values(const expr* for_child,
                                                                          range parent_values) const {
    return parent_values;
}

template <typename FromType>
using zero_extend_expr = extend_expr<false, FromType>;

template <typename FromType>
using sign_extend_expr = extend_expr<true, FromType>;

using zero_extend32_expr = zero_extend_expr<uint32_t>;
using sign_extend32_expr = sign_extend_expr<int32_t>;

} /* namespace libexpr */

#endif /* LIBEXPR_INCLUDE_EXPR_EXTEND_HPP */