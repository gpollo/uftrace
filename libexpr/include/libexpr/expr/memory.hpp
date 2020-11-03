#ifndef LIBEXPR_INCLUDE_EXPR_MEMORY_HPP
#define LIBEXPR_INCLUDE_EXPR_MEMORY_HPP

#include <limits>
#include <typeinfo>

#include "libexpr/context.hpp"
#include "libexpr/expr/child/one.hpp"
#include "libexpr/utils.hpp"

namespace libexpr {

template <unsigned int AddrSize, typename WordType>
class memory_expr : public one_child_expr {
   public:
    memory_expr(context& context, expr_ptr child);
    virtual ~memory_expr();
    virtual expr_ptr deep_copy() const;

    virtual std::string get_name() const;
    virtual std::optional<int128_t> evaluate_impl(int128_t child_result) const;

   private:
    virtual std::optional<range> get_possible_values(const expr* for_child, range parent_values) const;
};

template <unsigned int AddrSize, typename WordType>
memory_expr<AddrSize, WordType>::memory_expr(context& context, expr_ptr child) : one_child_expr(context, child) {}

template <unsigned int AddrSize, typename WordType>
memory_expr<AddrSize, WordType>::~memory_expr() {}

template <unsigned int AddrSize, typename WordType>
expr_ptr memory_expr<AddrSize, WordType>::deep_copy() const {
    auto child = child_->deep_copy();
    return context_.make_expr<memory_expr<AddrSize, WordType>>(child);
}

template <unsigned int AddrSize, typename WordType>
std::string memory_expr<AddrSize, WordType>::get_name() const {
    return "memory-" + std::to_string(AddrSize) + "-" + utils::type_name<WordType>::name;
}

template <unsigned int AddrSize>
static constexpr uint64_t bits_max_value() {
    uint64_t max = 1;

    for (unsigned int i = 0; i < AddrSize; i++) {
        max *= 2;
    }

    return max - 1;
}

template <unsigned int AddrSize, typename WordType>
std::optional<int128_t> memory_expr<AddrSize, WordType>::evaluate_impl(int128_t child_result) const {
    static_assert(bits_max_value<AddrSize>() <= std::numeric_limits<uint64_t>::max());

    auto addr = child_result;
    if (addr <= 0 || bits_max_value<AddrSize>() < addr) {
        return {};
    }

    WordType result;
    if (!context_.get_memory().read(addr, result)) {
        return {};
    }

    return static_cast<int128_t>(result);
}

template <unsigned int AddrSize, typename WordType>
std::optional<range> memory_expr<AddrSize, WordType>::get_possible_values(const expr* for_child,
                                                                          range parent_values) const {
    return range();
}

template <typename WordType>
using memory32_expr = memory_expr<32, WordType>;

template <typename WordType>
using memory64_expr = memory_expr<64, WordType>;

} /* namespace libexpr */

#endif /* LIBEXPR_INCLUDE_EXPR_MEMORY_HPP */