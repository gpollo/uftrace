#include <cassert>
#include <iostream>
#include <vector>

#include "libexpr/context.hpp"
#include "libexpr/expr/bound.hpp"
#include "libexpr/expr/register.hpp"
#include "libexpr/expr/root.hpp"

namespace libexpr {

context::context(memory_interface& memory) : memory_(memory) {}

const memory_interface& context::get_memory() const {
    return memory_;
}

std::optional<int128_t> context::get_value(const_expr_ptr expr) const {
    auto it = values_.find(expr);
    if (it == values_.end()) {
        return {};
    }

    return it->second;
}

void context::set_value(expr_ptr expr, int128_t value) {
    /* TODO: can be replaced by C++17's std::unordered_map<K,V>::insert_or_assign */

    auto it = values_.find(expr);
    if (it == values_.end()) {
        values_.insert({expr, value});
    } else {
        values_.erase(it);
        values_.insert({expr, value});
    }
}

const expr_ptr context::get_root_expr() const {
    return root_expr_;
}

size_t context::get_expr_count() const {
    size_t count = 0;

    for (auto pair : exprs_) {
        count += pair.second.size();
    }

    return count;
}

bool context::add_expr(expr_ptr expr) {
    if (typeid(*expr) == typeid(root_expr)) {
        if (root_expr_ == nullptr) {
            root_expr_ = expr;
        } else {
            std::cerr << "error: root is already set" << std::endl;
            return false;
        }
    }

    if (root_expr_ == nullptr) {
        std::cerr << "error: root is not set" << std::endl;
        return false;
    }

    if (exprs_.find(std::type_index(typeid(*expr))) == exprs_.end()) {
        exprs_.insert({std::type_index(typeid(*expr)), std::unordered_set<expr_ptr>()});
    }

    auto it = exprs_.find(std::type_index(typeid(*expr)));
    it->second.insert(expr);

    for (auto child : expr->get_children()) {
        add_expr(child);
    }

    return true;
}

void context::remove_expr(expr_ptr expr) {
    auto it_map = exprs_.find(std::type_index(typeid(*expr)));
    if (it_map == exprs_.end()) {
        return;
    }

    auto& set = it_map->second;

    auto it_set = set.begin();
    while (it_set != set.end()) {
        if (expr->equal_to(*it_set)) {
            it_set = set.erase(it_set);
        } else {
            it_set++;
        }
    }

    for (auto child : expr->get_children()) {
        remove_expr(child);
    }
}

size_t context::resolve_expr(expr_ptr target, expr_ptr replacement) {
    auto it_map = exprs_.find(std::type_index(typeid(*target)));
    if (it_map == exprs_.end()) {
        return 0;
    }

    auto& set = it_map->second;

    std::vector<expr_ptr> equal_exprs;
    for (auto contained_expr : set) {
        if (target->equal_to(contained_expr)) {
            equal_exprs.push_back(contained_expr);
        }
    }

    if (equal_exprs.size() == 0) {
        return 0;
    }

    bool has_parent_changed = false;
    for (auto expr : equal_exprs) {
        remove_expr(expr);

        auto parent = expr->get_parent();
        if (parent == nullptr) {
            continue;
        }

        bool succeeded;
        if (!has_parent_changed) {
            has_parent_changed = true;
            succeeded          = parent->replace_child(expr, replacement);
        } else {
            succeeded = parent->replace_child(expr, replacement->deep_copy());
        }

        if (!succeeded) {
            std::cerr << "malformed tree detected" << std::endl;
        }
    }

    if (equal_exprs.size() >= 1) {
        add_expr(replacement);
    }

    return equal_exprs.size();
}

bool context::is_there_any_unbounded_register() const {
    auto it_map = exprs_.find(std::type_index(typeid(register_expr)));
    if (it_map == exprs_.end()) {
        return false;
    }

    for (auto reg_expr : it_map->second) {
        expr_ptr parent = reg_expr->get_parent();

        while (true) {
            if (parent == nullptr) {
                return true;
            }

            if (typeid(*parent) == typeid(upper_bound_expr)) {
                return false;
            }

            parent = parent->get_parent();
        }
    }

    return false;
}

std::vector<expr_ptr> context::get_unbounded_registers() const {
    auto it_map = exprs_.find(std::type_index(typeid(register_expr)));
    if (it_map == exprs_.end()) {
        return {};
    }

    std::vector<expr_ptr> reg_exprs;
    for (auto reg_expr : it_map->second) {
        expr_ptr parent = reg_expr->get_parent();

        while (true) {
            if (parent == nullptr) {
                reg_exprs.push_back(reg_expr);
                break;
            }

            if (typeid(*parent) == typeid(upper_bound_expr)) {
                break;
            }

            parent = parent->get_parent();
        }
    }

    return reg_exprs;
}

std::vector<std::string> context::get_register_names() const {
    auto it_map = exprs_.find(std::type_index(typeid(register_expr)));
    if (it_map == exprs_.end()) {
        return {};
    }

    std::vector<std::string> names;
    for (auto expr : it_map->second) {
        auto reg_expr = std::dynamic_pointer_cast<libexpr::register_expr>(expr);
        names.push_back(reg_expr->get_register_name());
    }

    return names;
}

} /* namespace libexpr */
