#ifndef LIBEXPR_CONTEXT_HPP
#define LIBEXPR_CONTEXT_HPP

#include <cstdint>
#include <map>
#include <set>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>

#include "libexpr/expr/expr.hpp"
#include "libexpr/range.hpp"

namespace libexpr {

class memory_interface {
   public:
    virtual ~memory_interface() = default;

    virtual bool read(uint64_t addr, uint8_t& value) const  = 0;
    virtual bool read(uint64_t addr, uint16_t& value) const = 0;
    virtual bool read(uint64_t addr, uint32_t& value) const = 0;
    virtual bool read(uint64_t addr, uint64_t& value) const = 0;

    virtual bool read(uint64_t addr, int8_t& value) const  = 0;
    virtual bool read(uint64_t addr, int16_t& value) const = 0;
    virtual bool read(uint64_t addr, int32_t& value) const = 0;
    virtual bool read(uint64_t addr, int64_t& value) const = 0;
};

class register_expr;

class context {
   public:
    context(memory_interface& memory);

    const memory_interface& get_memory() const;

    std::optional<int128_t> get_value(const_expr_ptr expr) const;
    void set_value(expr_ptr expr, int128_t value);

    const expr_ptr get_root_expr() const;
    size_t get_expr_count() const;

    bool add_expr(expr_ptr expr);
    void remove_expr(expr_ptr expr);
    size_t resolve_expr(expr_ptr target, expr_ptr replacement);

    bool is_there_any_unbounded_register() const;
    std::vector<expr_ptr> get_unbounded_registers() const;
    std::vector<std::string> get_register_names() const;
    std::optional<std::vector<range>> get_outermost_bounds() const;

    template <typename ExprType, typename... Args>
    std::shared_ptr<ExprType> make_expr(Args&&... args);

    template <typename ExprType>
    const std::vector<std::shared_ptr<ExprType>> get_exprs() const;

   private:
    memory_interface& memory_;
    std::unordered_map<const_expr_ptr, int128_t> values_;
    std::unordered_map<std::type_index, std::unordered_set<expr_ptr>> exprs_;
    expr_ptr root_expr_ = nullptr;
};

template <typename ExprType, typename... Args>
std::shared_ptr<ExprType> context::make_expr(Args&&... args) {
    auto expr = std::make_shared<ExprType>(*this, args...);
    expr->init_children();
    return expr;
}

template <typename ExprType>
const std::vector<std::shared_ptr<ExprType>> context::get_exprs() const {
    auto it_map = exprs_.find(std::type_index(typeid(ExprType)));
    if (it_map == exprs_.end()) {
        return {};
    }

    std::vector<std::shared_ptr<ExprType>> exprs;
    for (auto expr : it_map->second) {
        exprs.push_back(std::dynamic_pointer_cast<ExprType>(expr));
    }

    return exprs;
}

} /* namespace libexpr */

#endif /* LIBEXPR_CONTEXT_HPP */