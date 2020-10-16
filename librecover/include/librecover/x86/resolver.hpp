#ifndef LIBRECOVER_INCLUDE_X86_RESOLVER_HPP
#define LIBRECOVER_INCLUDE_X86_RESOLVER_HPP

#include <cstdint>
#include <optional>
#include <vector>

#include "capstone/capstone.h"
#include "capstone/x86.h"
#include "libexpr/context.hpp"

namespace librecover {

namespace x86 {

class resolver {
   public:
    resolver(csh handle, std::vector<cs_insn*> insns, libexpr::memory_interface& memory, bool debug = false);

    std::optional<std::vector<uint64_t>> get_indirect_jump_targets(size_t index);

   private:
    csh handle_;
    std::vector<cs_insn*> insns_;
    libexpr::memory_interface& memory_;
    bool debug_ = false;

    bool is_jump(cs_insn& insn) const;
    bool is_indirect_jump(cs_insn& insn) const;

    bool add_root_node(libexpr::context& ctx, cs_insn& insn) const;
    bool rewind_instructions(libexpr::context& ctx, size_t index);

    bool try_process_mov_reg_reg_instruction(libexpr::context& ctx, cs_insn& insn) const;

    bool process_instruction(libexpr::context& ctx, cs_insn& insn, size_t index);
    bool process_add_instruction(libexpr::context& ctx, cs_insn& insn) const;
    bool process_mov_instruction(libexpr::context& ctx, cs_insn& insn) const;
    bool process_movsxd_instruction(libexpr::context& ctx, cs_insn& insn) const;
    bool process_lea_instruction(libexpr::context& ctx, cs_insn& insn) const;
    bool process_cdqe_instruction(libexpr::context& ctx, cs_insn& insn) const;
    bool process_ja_instruction(libexpr::context& ctx, size_t index) const;

    std::optional<std::unordered_set<x86_reg>> get_context_registers(libexpr::context& ctx) const;
    std::optional<std::unordered_set<x86_reg>> get_insn_implicit_read_registers(cs_insn& insn) const;
    std::optional<std::unordered_set<x86_reg>> get_insn_implicit_write_registers(cs_insn& insn) const;
    std::optional<std::unordered_set<x86_reg>> get_insn_read_registers(cs_insn& insn) const;
    std::optional<std::unordered_set<x86_reg>> get_insn_write_registers(cs_insn& insn) const;
    std::optional<std::unordered_set<uint64_t>> get_insn_eflags(uint64_t eflags) const;

    std::optional<bool> does_write_eflags(cs_insn& insn) const;
    std::optional<bool> are_test_flags_modified(const std::vector<uint64_t>& test_flags,
                                                const std::unordered_set<uint64_t>& insn_flags) const;

    std::optional<size_t> find_backward_insn_that_modifies_flags(size_t index, std::vector<uint64_t> test_eflags) const;
    std::optional<size_t> find_backward_insn_with_registers_or_jmp(libexpr::context& ctx, size_t index) const;

    template <typename T>
    libexpr::expr_ptr create_expr(libexpr::context& ctx, libexpr::expr_ptr c) const;

    template <typename T>
    libexpr::expr_ptr create_expr(libexpr::context& ctx, libexpr::expr_ptr c1, libexpr::expr_ptr c2) const;

    libexpr::expr_ptr create_constant_expr(libexpr::context& ctx, int64_t value) const;
    libexpr::expr_ptr create_register_expr(libexpr::context& ctx, x86_reg reg) const;

    libexpr::expr_ptr create_operand_expr(libexpr::context& ctx, cs_insn& insn, cs_x86_op& operand) const;
    libexpr::expr_ptr create_register_operand_expr(libexpr::context& ctx, cs_insn& insn, x86_reg& reg) const;
    libexpr::expr_ptr create_immediate_operand_expr(libexpr::context& ctx, int64_t imm) const;
    libexpr::expr_ptr create_memory_operand_expr(libexpr::context& ctx, cs_insn& insn, x86_op_mem& mem) const;
};

template <typename T>
libexpr::expr_ptr resolver::create_expr(libexpr::context& ctx, libexpr::expr_ptr c) const {
    if (c == nullptr) {
        std::cout << "error: nullptr received" << std::endl;
        return nullptr;
    }

    return ctx.make_expr<T>(c);
}

template <typename T>
libexpr::expr_ptr resolver::create_expr(libexpr::context& ctx, libexpr::expr_ptr c1, libexpr::expr_ptr c2) const {
    if (c1 == nullptr || c2 == nullptr) {
        std::cout << "error: nullptr received" << std::endl;
        return nullptr;
    }

    return ctx.make_expr<T>(c1, c2);
}

} /* namespace x86 */

} /* namespace librecover */

#endif /* LIBRECOVER_INCLUDE_X86_RESOLVER_HPP */