#include <cassert>
#include <unordered_map>
#include <unordered_set>

#include "libexpr/expr/bound.hpp"
#include "libexpr/expr/constant.hpp"
#include "libexpr/expr/extend.hpp"
#include "libexpr/expr/memory.hpp"
#include "libexpr/expr/operation.hpp"
#include "libexpr/expr/register.hpp"
#include "libexpr/expr/root.hpp"
#include "libexpr/impl/process_memory.hpp"
#include "librecover/utils.hpp"
#include "librecover/x86/resolver.h"
#include "librecover/x86/resolver.hpp"

static const std::unordered_set<x86_insn> X86_JUMP_IDS = {
    X86_INS_JAE, X86_INS_JA,  X86_INS_JBE, X86_INS_JB, X86_INS_JCXZ,  X86_INS_JECXZ, X86_INS_JE,
    X86_INS_JGE, X86_INS_JG,  X86_INS_JLE, X86_INS_JL, X86_INS_JMP,   X86_INS_JNE,   X86_INS_JNO,
    X86_INS_JNP, X86_INS_JNS, X86_INS_JO,  X86_INS_JP, X86_INS_JRCXZ, X86_INS_JS,
};

static const std::unordered_set<x86_insn> X86_CMP_IDS = {
    X86_INS_CMP,
    X86_INS_CMPSB,
    X86_INS_CMPSQ,
    X86_INS_CMPSW,
};

static const std::unordered_set<x86_reg> X86_INSN_PTR_REGISTERS = {
    X86_REG_IP,
    X86_REG_EIP,
    X86_REG_RIP,
};

static const std::unordered_map<x86_reg, std::string> X86_64BITS_REGISTERS_NAME = {
    {X86_REG_RAX, "rax"}, {X86_REG_RBX, "rbx"}, {X86_REG_RCX, "rcx"}, {X86_REG_RDX, "rdx"},
    {X86_REG_RSP, "rsp"}, {X86_REG_RBP, "rbp"}, {X86_REG_RDI, "rdi"}, {X86_REG_RSI, "rsi"},
    {X86_REG_R8, "r8"},   {X86_REG_R9, "r9"},   {X86_REG_R10, "r10"}, {X86_REG_R11, "r11"},
    {X86_REG_R12, "r12"}, {X86_REG_R13, "r13"}, {X86_REG_R14, "r14"}, {X86_REG_R15, "r15"},
};

static const std::unordered_map<std::string, x86_reg> X86_64BITS_REGISTERS_FROM_NAME = {
    {"rax", X86_REG_RAX}, {"rbx", X86_REG_RBX}, {"rcx", X86_REG_RCX}, {"rdx", X86_REG_RDX},
    {"rsp", X86_REG_RSP}, {"rbp", X86_REG_RBP}, {"rdi", X86_REG_RDI}, {"rsi", X86_REG_RSI},
    {"r8", X86_REG_R8},   {"r9", X86_REG_R9},   {"r10", X86_REG_R10}, {"r11", X86_REG_R11},
    {"r12", X86_REG_R12}, {"r13", X86_REG_R13}, {"r14", X86_REG_R14}, {"r15", X86_REG_R15},
};

static const std::unordered_set<x86_reg> X86_OTHER_REGISTERS = {
    X86_REG_EFLAGS,
    X86_REG_IP,
    X86_REG_EIP,
    X86_REG_RIP,
};

static const std::unordered_set<x86_reg> X86_64BITS_REGISTERS = {
    X86_REG_RAX, X86_REG_RBX, X86_REG_RCX, X86_REG_RDX, X86_REG_RSP, X86_REG_RBP, X86_REG_RDI, X86_REG_RSI,
    X86_REG_R8,  X86_REG_R9,  X86_REG_R10, X86_REG_R11, X86_REG_R12, X86_REG_R13, X86_REG_R14, X86_REG_R15,
};

static const std::unordered_map<x86_reg, x86_reg> X86_32BITS_REGISTERS = {
    {X86_REG_EAX, X86_REG_RAX},  {X86_REG_EBX, X86_REG_RBX},  {X86_REG_ECX, X86_REG_RCX},  {X86_REG_EDX, X86_REG_RDX},
    {X86_REG_ESP, X86_REG_RSP},  {X86_REG_EBP, X86_REG_RBP},  {X86_REG_EDI, X86_REG_RDI},  {X86_REG_ESI, X86_REG_RSI},
    {X86_REG_R8D, X86_REG_R8},   {X86_REG_R9D, X86_REG_R9},   {X86_REG_R10D, X86_REG_R10}, {X86_REG_R11D, X86_REG_R11},
    {X86_REG_R12D, X86_REG_R12}, {X86_REG_R13D, X86_REG_R13}, {X86_REG_R14D, X86_REG_R14}, {X86_REG_R15D, X86_REG_R15},
};

static const std::unordered_map<x86_reg, x86_reg> X86_16BITS_REGISTERS = {
    {X86_REG_AX, X86_REG_RAX},   {X86_REG_BX, X86_REG_RBX},   {X86_REG_CX, X86_REG_RCX},   {X86_REG_DX, X86_REG_RDX},
    {X86_REG_SP, X86_REG_RSP},   {X86_REG_BP, X86_REG_RBP},   {X86_REG_DI, X86_REG_RDI},   {X86_REG_SI, X86_REG_RSI},
    {X86_REG_R8W, X86_REG_R8},   {X86_REG_R9W, X86_REG_R9},   {X86_REG_R10W, X86_REG_R10}, {X86_REG_R11W, X86_REG_R11},
    {X86_REG_R12W, X86_REG_R12}, {X86_REG_R13W, X86_REG_R13}, {X86_REG_R14W, X86_REG_R14}, {X86_REG_R15W, X86_REG_R15},
};

static const std::unordered_map<x86_reg, x86_reg> X86_8BITS_REGISTERS = {
    {X86_REG_AL, X86_REG_RAX},   {X86_REG_BL, X86_REG_RBX},   {X86_REG_CL, X86_REG_RCX},   {X86_REG_DL, X86_REG_RDX},
    {X86_REG_SPL, X86_REG_RSP},  {X86_REG_BPL, X86_REG_RBP},  {X86_REG_DIL, X86_REG_RDI},  {X86_REG_SIL, X86_REG_RSI},
    {X86_REG_R8B, X86_REG_R8},   {X86_REG_R9B, X86_REG_R9},   {X86_REG_R10B, X86_REG_R10}, {X86_REG_R11B, X86_REG_R11},
    {X86_REG_R12B, X86_REG_R12}, {X86_REG_R13B, X86_REG_R13}, {X86_REG_R14B, X86_REG_R14}, {X86_REG_R15B, X86_REG_R15},
};

static const std::unordered_map<uint64_t, uint64_t> X86_EFLAGS_TEST_TO_MODIFY = {
    {X86_EFLAGS_TEST_OF, X86_EFLAGS_MODIFY_OF}, {X86_EFLAGS_TEST_SF, X86_EFLAGS_MODIFY_SF},
    {X86_EFLAGS_TEST_ZF, X86_EFLAGS_MODIFY_ZF}, {X86_EFLAGS_TEST_PF, X86_EFLAGS_MODIFY_PF},
    {X86_EFLAGS_TEST_CF, X86_EFLAGS_MODIFY_CF}, {X86_EFLAGS_TEST_NT, X86_EFLAGS_MODIFY_NT},
    {X86_EFLAGS_TEST_DF, X86_EFLAGS_MODIFY_DF}, {X86_EFLAGS_TEST_RF, X86_EFLAGS_MODIFY_RF},
    {X86_EFLAGS_TEST_IF, X86_EFLAGS_MODIFY_IF}, {X86_EFLAGS_TEST_AF, X86_EFLAGS_MODIFY_AF},
};

static const std::unordered_map<uint64_t, uint64_t> X86_EFLAGS_TEST_TO_RESET = {
    {X86_EFLAGS_TEST_OF, X86_EFLAGS_RESET_OF}, {X86_EFLAGS_TEST_SF, X86_EFLAGS_RESET_SF},
    {X86_EFLAGS_TEST_ZF, X86_EFLAGS_RESET_ZF}, {X86_EFLAGS_TEST_PF, X86_EFLAGS_RESET_PF},
    {X86_EFLAGS_TEST_CF, X86_EFLAGS_RESET_CF}, {X86_EFLAGS_TEST_NT, X86_EFLAGS_RESET_NT},
    {X86_EFLAGS_TEST_DF, X86_EFLAGS_RESET_DF}, {X86_EFLAGS_TEST_RF, X86_EFLAGS_RESET_RF},
    {X86_EFLAGS_TEST_IF, X86_EFLAGS_RESET_IF}, {X86_EFLAGS_TEST_TF, X86_EFLAGS_RESET_TF},
    {X86_EFLAGS_TEST_AF, X86_EFLAGS_RESET_AF},
};

static const std::unordered_map<uint64_t, uint64_t> X86_EFLAGS_TEST_TO_SET = {
    {X86_EFLAGS_TEST_OF, X86_EFLAGS_SET_OF}, {X86_EFLAGS_TEST_SF, X86_EFLAGS_SET_SF},
    {X86_EFLAGS_TEST_ZF, X86_EFLAGS_SET_ZF}, {X86_EFLAGS_TEST_PF, X86_EFLAGS_SET_PF},
    {X86_EFLAGS_TEST_CF, X86_EFLAGS_SET_CF}, {X86_EFLAGS_TEST_DF, X86_EFLAGS_SET_DF},
    {X86_EFLAGS_TEST_IF, X86_EFLAGS_SET_IF}, {X86_EFLAGS_TEST_AF, X86_EFLAGS_SET_AF},
};

static const std::unordered_set<uint64_t> X86_EFLAGS = {
    X86_EFLAGS_MODIFY_AF,    X86_EFLAGS_MODIFY_CF,    X86_EFLAGS_MODIFY_SF,    X86_EFLAGS_MODIFY_ZF,
    X86_EFLAGS_MODIFY_PF,    X86_EFLAGS_MODIFY_OF,    X86_EFLAGS_MODIFY_TF,    X86_EFLAGS_MODIFY_IF,
    X86_EFLAGS_MODIFY_DF,    X86_EFLAGS_MODIFY_NT,    X86_EFLAGS_MODIFY_RF,    X86_EFLAGS_PRIOR_OF,
    X86_EFLAGS_PRIOR_SF,     X86_EFLAGS_PRIOR_ZF,     X86_EFLAGS_PRIOR_AF,     X86_EFLAGS_PRIOR_PF,
    X86_EFLAGS_PRIOR_CF,     X86_EFLAGS_PRIOR_TF,     X86_EFLAGS_PRIOR_IF,     X86_EFLAGS_PRIOR_DF,
    X86_EFLAGS_PRIOR_NT,     X86_EFLAGS_RESET_OF,     X86_EFLAGS_RESET_CF,     X86_EFLAGS_RESET_DF,
    X86_EFLAGS_RESET_IF,     X86_EFLAGS_RESET_SF,     X86_EFLAGS_RESET_AF,     X86_EFLAGS_RESET_TF,
    X86_EFLAGS_RESET_NT,     X86_EFLAGS_RESET_PF,     X86_EFLAGS_SET_CF,       X86_EFLAGS_SET_DF,
    X86_EFLAGS_SET_IF,       X86_EFLAGS_TEST_OF,      X86_EFLAGS_TEST_SF,      X86_EFLAGS_TEST_ZF,
    X86_EFLAGS_TEST_PF,      X86_EFLAGS_TEST_CF,      X86_EFLAGS_TEST_NT,      X86_EFLAGS_TEST_DF,
    X86_EFLAGS_UNDEFINED_OF, X86_EFLAGS_UNDEFINED_SF, X86_EFLAGS_UNDEFINED_ZF, X86_EFLAGS_UNDEFINED_PF,
    X86_EFLAGS_UNDEFINED_AF, X86_EFLAGS_UNDEFINED_CF, X86_EFLAGS_RESET_RF,     X86_EFLAGS_TEST_RF,
    X86_EFLAGS_TEST_IF,      X86_EFLAGS_TEST_TF,      X86_EFLAGS_TEST_AF,      X86_EFLAGS_RESET_ZF,
    X86_EFLAGS_SET_OF,       X86_EFLAGS_SET_SF,       X86_EFLAGS_SET_ZF,       X86_EFLAGS_SET_AF,
    X86_EFLAGS_SET_PF,       X86_EFLAGS_RESET_0F,     X86_EFLAGS_RESET_AC,
};

namespace librecover {

namespace x86 {

resolver::resolver(csh handle, std::vector<cs_insn*> insns, libexpr::memory_interface& memory, bool debug)
    : handle_(handle), insns_(insns), memory_(memory), debug_(debug) {}

std::optional<std::vector<uint64_t>> resolver::get_indirect_jump_targets(size_t index) {
    if (index >= insns_.size()) {
        return std::vector<uint64_t>();
    }

    if (insns_[index]->detail == nullptr) {
        std::cerr << "error: capstone instruction details is off" << std::endl;
        return {};
    }

    libexpr::context ctx(memory_);

    if (!add_root_node(ctx, *insns_[index])) {
        std::cerr << "error: failed to add root node" << std::endl;
        return {};
    }
    auto root_expr = ctx.get_root_expr();

    if (index == 0) {
        std::cerr << "error: no more instructions to rewind" << std::endl;
        return {};
    }

    if (!rewind_instructions(ctx, index)) {
        std::cerr << "error: instructions rewinding failed" << std::endl;
        return {};
    }

    /* TODO: we assume there is always 1 upper bound */
    /* TODO: we assume there is no lower bounds */
    auto upper_bound_exprs = ctx.get_exprs<libexpr::upper_bound_expr>();
    if (upper_bound_exprs.size() != 1) {
        std::cerr << "error: expected 1 upper bound, got " << upper_bound_exprs.size() << std::endl;
        return {};
    }

    auto upper_bound_expr = std::dynamic_pointer_cast<libexpr::expr>(upper_bound_exprs[0]);
    auto range_opt        = upper_bound_expr->get_possible_values(upper_bound_expr->get_children()[0].get());
    if (!range_opt.has_value()) {
        std::cerr << "error: failed to obtain possible values of upper bound" << std::endl;
        return {};
    }
    auto range = range_opt.value();

    auto maximum_opt = range.get_finite_maximum();
    if (!maximum_opt.has_value()) {
        std::cerr << "error: range doesn't have a maximum" << std::endl;
        return {};
    }
    auto maximum = maximum_opt.value();

    /* TODO: we assume that the range of value is [0, max[ */
    std::vector<uint64_t> results;
    for (auto i = 0; i <= maximum; i++) {
        ctx.set_value(upper_bound_expr, i);

        auto result_opt = root_expr->evaluate();
        if (!result_opt.has_value()) {
            std::cerr << "error: failed to evaluate expression tree" << std::endl;
            return {};
        }
        auto result = result_opt.value();

        results.push_back(result);
    }

    return results;
}

bool resolver::is_jump(cs_insn& insn) const {
    return (X86_JUMP_IDS.find(static_cast<x86_insn>(insn.id)) != X86_JUMP_IDS.end());
}

bool resolver::is_indirect_jump(cs_insn& insn) const {
    if (!is_jump(insn)) {
        return false;
    }

    cs_x86* x86 = &insn.detail->x86;
    if (x86->op_count != 1) {
        std::cerr << "error: capstone jump instruction doesn't have one operand" << std::endl;
        return false;
    }

    cs_x86_op* operand = &x86->operands[0];
    if (operand->type != X86_OP_IMM) {
        return true;
    }

    return false;
}

bool resolver::add_root_node(libexpr::context& ctx, cs_insn& insn) const {
    if (!is_indirect_jump(insn)) {
        std::cerr << "error: instruction is not an indirect jump" << std::endl;
        return false;
    }

    if (ctx.get_root_expr() != nullptr) {
        std::cerr << "error: root node is already set" << std::endl;
        return false;
    }

    if (debug_) {
        std::cerr << "debug: " << insn.mnemonic << " " << insn.op_str << std::endl;
    }

    auto& operand     = insn.detail->x86.operands[0];
    auto operand_expr = create_operand_expr(ctx, insn, operand);
    auto root_expr    = create_expr<libexpr::root_expr>(ctx, operand_expr);
    if (root_expr == nullptr) {
        std::cerr << "error: failed to create root node" << std::endl;
        return false;
    }

    if (!ctx.add_expr(root_expr)) {
        std::cerr << "error: failed to add root node" << std::endl;
        return false;
    }

    return true;
}

bool resolver::rewind_instructions(libexpr::context& ctx, size_t index) {
    while (true) {
        if (debug_) {
            auto expr = std::dynamic_pointer_cast<libexpr::expr>(ctx.get_root_expr());
            std::cerr << "=== debug tree start ===" << std::endl;
            std::cerr << expr;
            std::cerr << "==== debug tree end ====" << std::endl;
        }

        auto next_index_opt = find_backward_insn_with_registers_or_jmp(ctx, index);
        if (!next_index_opt.has_value()) {
            std::cerr << "error: failed to find instruction" << std::endl;
            return false;
        }
        auto next_index = next_index_opt.value();

        auto& insn = *insns_[next_index];
        if (!process_instruction(ctx, insn, next_index)) {
            std::cerr << "error: failed to process instruction" << std::endl;
            return false;
        }

        auto unbounded_registers = ctx.get_unbounded_registers();
        if (unbounded_registers.size() == 0) {
            break;
        }

        index = next_index;
        if (index == 0) {
            std::cerr << "error: reached the end of instructions" << std::endl;
            return false;
        }
    }

    if (debug_) {
        auto expr = std::dynamic_pointer_cast<libexpr::expr>(ctx.get_root_expr());
        std::cerr << "=== debug tree start ===" << std::endl;
        std::cerr << expr;
        std::cerr << "==== debug tree end ====" << std::endl;
    }

    return true;
}

bool resolver::process_instruction(libexpr::context& ctx, cs_insn& insn, size_t index) {
    if (debug_) {
        std::cerr << "debug: " << insn.mnemonic << " " << insn.op_str << std::endl;
    }

    switch (insn.id) {
    case X86_INS_ADD:
        return process_add_instruction(ctx, insn);

    case X86_INS_MOV:
        return process_mov_instruction(ctx, insn);

    case X86_INS_MOVSXD:
        return process_movsxd_instruction(ctx, insn);

    case X86_INS_LEA:
        return process_lea_instruction(ctx, insn);

    case X86_INS_CDQE:
        return process_cdqe_instruction(ctx, insn);

    case X86_INS_JA:
        return process_ja_instruction(ctx, index);

    default:
        std::cerr << "error: mnemonic not implemented `" << insn.mnemonic << "`" << std::endl;
        return false;
    }
}

bool resolver::try_process_mov_reg_reg_instruction(libexpr::context& ctx, cs_insn& insn) const {
    assert(insn.detail != nullptr);
    assert(insn.detail->x86.op_count == 2);

    auto x86  = insn.detail->x86;
    auto& op1 = x86.operands[0];
    auto& op2 = x86.operands[1];

    if (op1.type != op2.type || op1.type != X86_OP_REG || op1.reg != op2.reg) {
        return false;
    }

    auto reg = op1.reg;
    if (X86_32BITS_REGISTERS.find(reg) == X86_32BITS_REGISTERS.end()) {
        return false;
    }

    auto dst_expr        = create_operand_expr(ctx, insn, op1);
    auto src_expr        = create_operand_expr(ctx, insn, op1);
    auto src_extend_expr = create_expr<libexpr::zero_extend32_expr>(ctx, src_expr);

    return (ctx.resolve_expr(dst_expr, src_extend_expr) > 0);
}

bool resolver::process_add_instruction(libexpr::context& ctx, cs_insn& insn) const {
    assert(insn.detail != nullptr);
    assert(insn.detail->x86.op_count == 2);

    auto x86          = insn.detail->x86;
    auto dst_expr     = create_operand_expr(ctx, insn, x86.operands[0]);
    auto src_op1_expr = create_operand_expr(ctx, insn, x86.operands[0]);
    auto src_op2_expr = create_operand_expr(ctx, insn, x86.operands[1]);
    auto src_add_expr = create_expr<libexpr::add_expr>(ctx, src_op1_expr, src_op2_expr);

    return (ctx.resolve_expr(dst_expr, src_add_expr) > 0);
}

bool resolver::process_mov_instruction(libexpr::context& ctx, cs_insn& insn) const {
    assert(insn.detail != nullptr);
    assert(insn.detail->x86.op_count == 2);

    if (try_process_mov_reg_reg_instruction(ctx, insn)) {
        return true;
    }

    auto x86      = insn.detail->x86;
    auto& op1     = x86.operands[0];
    auto& op2     = x86.operands[1];
    auto dst_expr = create_operand_expr(ctx, insn, op1);
    auto src_expr = create_operand_expr(ctx, insn, op2);

    if (op1.type == X86_OP_MEM) {
        dst_expr = create_expr<libexpr::memory32_expr<int32_t>>(ctx, dst_expr);
    }

    if (op2.type == X86_OP_MEM) {
        src_expr = create_expr<libexpr::memory32_expr<int32_t>>(ctx, src_expr);
    }

    return (ctx.resolve_expr(dst_expr, src_expr) > 0);
}

bool resolver::process_movsxd_instruction(libexpr::context& ctx, cs_insn& insn) const {
    assert(insn.detail != nullptr);
    assert(insn.detail->x86.op_count == 2);

    auto x86      = insn.detail->x86;
    auto& op1     = x86.operands[0];
    auto& op2     = x86.operands[1];
    auto dst_expr = create_operand_expr(ctx, insn, op1);
    auto src_expr = create_operand_expr(ctx, insn, op2);

    if (op1.type == X86_OP_MEM) {
        dst_expr = create_expr<libexpr::memory32_expr<int32_t>>(ctx, dst_expr);
    }

    if (op2.type == X86_OP_MEM) {
        src_expr = create_expr<libexpr::memory32_expr<int32_t>>(ctx, src_expr);
    }

    src_expr = create_expr<libexpr::sign_extend32_expr>(ctx, src_expr);

    return (ctx.resolve_expr(dst_expr, src_expr) > 0);
}

bool resolver::process_lea_instruction(libexpr::context& ctx, cs_insn& insn) const {
    assert(insn.detail != nullptr);
    assert(insn.detail->x86.op_count == 2);

    auto x86      = insn.detail->x86;
    auto dst_expr = create_operand_expr(ctx, insn, x86.operands[0]);
    auto src_expr = create_operand_expr(ctx, insn, x86.operands[1]);

    return (ctx.resolve_expr(dst_expr, src_expr) > 0);
}

bool resolver::process_cdqe_instruction(libexpr::context& ctx, cs_insn& insn) const {
    assert(insn.detail != nullptr);
    assert(insn.detail->x86.op_count == 0);

    auto dst_expr = create_register_expr(ctx, X86_REG_RAX);
    auto src_expr = create_register_expr(ctx, X86_REG_RAX);
    src_expr      = create_expr<libexpr::zero_extend32_expr>(ctx, src_expr);

    return (ctx.resolve_expr(dst_expr, src_expr) > 0);
}

bool resolver::process_ja_instruction(libexpr::context& ctx, size_t index) const {
    assert(1 <= index && index < insns_.size());
    assert(insns_[index]->detail != nullptr);
    assert(insns_[index]->detail->x86.op_count == 1);

    std::vector<uint64_t> test_eflags{X86_EFLAGS_TEST_ZF, X86_EFLAGS_TEST_CF};
    auto cmp_index_opt = find_backward_insn_that_modifies_flags(index, test_eflags);
    if (!cmp_index_opt.has_value()) {
        std::cerr << "error: failed to find instruction that modifies flags" << std::endl;
        return false;
    }

    auto& cmp_insn = *insns_[cmp_index_opt.value()];
    if (X86_CMP_IDS.find((x86_insn)cmp_insn.id) == X86_CMP_IDS.end()) {
        std::cerr << "error: expected compare instruction, got id `" << cmp_insn.id << "`" << std::endl;
        return false;
    }

    /* TODO: maybe create a function `process_cmp()` ? */

    auto x86                  = cmp_insn.detail->x86;
    auto& op1                 = x86.operands[0];
    auto& op2                 = x86.operands[1];
    auto dst_expr             = create_operand_expr(ctx, cmp_insn, op1);
    auto src_bounded_expr     = create_operand_expr(ctx, cmp_insn, op1);
    auto src_bound_value_expr = create_operand_expr(ctx, cmp_insn, op2);
    auto src_expr             = create_expr<libexpr::upper_bound_expr>(ctx, src_bounded_expr, src_bound_value_expr);

    if (op1.type == X86_OP_MEM) {
        dst_expr = create_expr<libexpr::memory32_expr<int32_t>>(ctx, dst_expr);
    }

    if (op2.type == X86_OP_MEM) {
        src_expr = create_expr<libexpr::memory32_expr<int32_t>>(ctx, src_expr);
    }

    return (ctx.resolve_expr(dst_expr, src_expr) > 0);
}

std::optional<std::unordered_set<x86_reg>> resolver::get_context_registers(libexpr::context& ctx) const {
    std::unordered_set<x86_reg> regs;
    for (auto& name : ctx.get_register_names()) {
        auto it = X86_64BITS_REGISTERS_FROM_NAME.find(name);
        if (it == X86_64BITS_REGISTERS_FROM_NAME.end()) {
            std::cerr << "error: unknown register `" << name << "`" << std::endl;
            return {};
        }

        regs.insert(it->second);
    }

    return regs;
}

static std::optional<std::unordered_set<x86_reg>> get_64bits_registers(cs_regs regs, uint8_t regs_count) {
    std::unordered_set<x86_reg> regs64;
    for (int i = 0; i < regs_count; i++) {
        auto reg = (x86_reg)regs[i];

        auto it_8 = X86_8BITS_REGISTERS.find(reg);
        if (it_8 != X86_8BITS_REGISTERS.end()) {
            regs64.insert(it_8->second);
            continue;
        }

        auto it_16 = X86_16BITS_REGISTERS.find(reg);
        if (it_16 != X86_16BITS_REGISTERS.end()) {
            regs64.insert(it_16->second);
            continue;
        }

        auto it_32 = X86_32BITS_REGISTERS.find(reg);
        if (it_32 != X86_32BITS_REGISTERS.end()) {
            regs64.insert(it_32->second);
            continue;
        }

        auto it_64 = X86_64BITS_REGISTERS.find(reg);
        if (it_64 != X86_64BITS_REGISTERS.end()) {
            regs64.insert(*it_64);
            continue;
        }

        auto it_other = X86_OTHER_REGISTERS.find(reg);
        if (it_other != X86_OTHER_REGISTERS.end()) {
            regs64.insert(*it_other);
            continue;
        }

        std::cerr << "error: unknown register `" << reg << "`" << std::endl;
        return {};
    }

    return regs64;
}

std::optional<std::unordered_set<x86_reg>> resolver::get_insn_implicit_read_registers(cs_insn& insn) const {
    return get_64bits_registers(insn.detail->regs_read, insn.detail->regs_read_count);
}

std::optional<std::unordered_set<x86_reg>> resolver::get_insn_implicit_write_registers(cs_insn& insn) const {
    return get_64bits_registers(insn.detail->regs_write, insn.detail->regs_write_count);
}

std::optional<std::unordered_set<x86_reg>> resolver::get_insn_read_registers(cs_insn& insn) const {
    cs_regs regs_read, regs_write;
    uint8_t regs_read_count, regs_write_count;
    if (cs_regs_access(handle_, &insn, regs_read, &regs_read_count, regs_write, &regs_write_count) != CS_ERR_OK) {
        std::cerr << "error: failed to access registers" << std::endl;
        return {};
    }

    return get_64bits_registers(regs_read, regs_read_count);
}

std::optional<std::unordered_set<x86_reg>> resolver::get_insn_write_registers(cs_insn& insn) const {
    cs_regs regs_read, regs_write;
    uint8_t regs_read_count, regs_write_count;
    if (cs_regs_access(handle_, &insn, regs_read, &regs_read_count, regs_write, &regs_write_count) != CS_ERR_OK) {
        std::cerr << "error: failed to access registers" << std::endl;
        return {};
    }

    return get_64bits_registers(regs_write, regs_write_count);
}

std::optional<std::unordered_set<uint64_t>> resolver::get_insn_eflags(uint64_t eflags) const {
    std::unordered_set<uint64_t> results;
    for (auto flag : X86_EFLAGS) {
        if ((eflags & flag) == 0) {
            continue;
        }

        results.insert(flag);

        eflags &= ~flag;
        if (eflags == 0) {
            break;
        }
    }

    if (eflags != 0) {
        std::cerr << "error: unknown eflags encountered `" << eflags << std::endl;
        return {};
    }

    return results;
}

std::optional<bool> resolver::does_write_eflags(cs_insn& insn) const {
    auto write_regs_opt = get_insn_implicit_write_registers(insn);
    if (!write_regs_opt.has_value()) {
        std::cerr << "error: failed to obtain write registers" << std::endl;
        return {};
    }
    auto write_regs = write_regs_opt.value();

    return (write_regs.find(X86_REG_EFLAGS) != write_regs.end());
}

std::optional<bool> resolver::are_test_flags_modified(const std::vector<uint64_t>& test_flags,
                                                      const std::unordered_set<uint64_t>& insn_flags) const {
    if (test_flags.size() == 0) {
        std::cerr << "error: no test flags specified" << std::endl;
        return {};
    }

    size_t count = 0;
    for (auto flag : test_flags) {
        auto it_mod = X86_EFLAGS_TEST_TO_MODIFY.find(flag);
        if (it_mod != X86_EFLAGS_TEST_TO_MODIFY.end()) {
            if (insn_flags.find(it_mod->second) != insn_flags.end()) {
                count++;
                continue;
            }
        }

        auto it_reset = X86_EFLAGS_TEST_TO_RESET.find(flag);
        if (it_reset != X86_EFLAGS_TEST_TO_RESET.end()) {
            if (insn_flags.find(it_mod->second) != insn_flags.end()) {
                count++;
                continue;
            }
        }

        auto it_set = X86_EFLAGS_TEST_TO_SET.find(flag);
        if (it_set != X86_EFLAGS_TEST_TO_SET.end()) {
            if (insn_flags.find(it_mod->second) != insn_flags.end()) {
                count++;
                continue;
            }
        }
    }

    if (count != test_flags.size()) {
        std::cerr << "error: missing modified flag" << std::endl;
        return {};
    }

    return (count != 0);
}

std::optional<size_t> resolver::find_backward_insn_that_modifies_flags(size_t index,
                                                                       std::vector<uint64_t> test_eflags) const {
    auto current_index = index;

    while (true) {
        if (current_index-- == 0) {
            std::cerr << "error: reached the end of instructions" << std::endl;
            return {};
        }

        auto& insn             = *insns_[current_index];
        auto writes_eflags_opt = does_write_eflags(insn);
        if (!writes_eflags_opt.has_value() || !writes_eflags_opt.value()) {
            continue;
        }

        auto insn_eflags_opt = get_insn_eflags(insn.detail->x86.eflags);
        if (!insn_eflags_opt.has_value()) {
            std::cerr << "error: failed to get eflags" << std::endl;
            return {};
        }
        auto insn_eflags = insn_eflags_opt.value();

        auto modified_opt = are_test_flags_modified(test_eflags, insn_eflags);
        if (!modified_opt.has_value()) {
            std::cerr << "error: failed to determine if eflags are modified" << std::endl;
            continue;
        }
        auto modified = modified_opt.value();

        if (modified) {
            return current_index;
        }
    }

    return {};
}

std::optional<size_t> resolver::find_backward_insn_with_registers_or_jmp(libexpr::context& ctx, size_t index) const {
    auto current_index = index;

    auto regs_opt = get_context_registers(ctx);
    if (!regs_opt.has_value()) {
        std::cerr << "error: failed to obtain context registers" << std::endl;
        return {};
    }
    auto& regs = *regs_opt;

    while (true) {
        if (current_index-- == 0) {
            std::cerr << "error: reached the end of instructions" << std::endl;
            return {};
        }

        auto& insn = *insns_[current_index];

        if (is_jump(insn)) {
            return current_index;
        }

        auto read_regs_opt = get_insn_read_registers(insn);
        if (!read_regs_opt.has_value()) {
            std::cerr << "error: failed to obtain read registers" << std::endl;
            return {};
        }
        auto read_regs = read_regs_opt.value();

        for (auto reg : read_regs) {
            auto it = regs.find(static_cast<x86_reg>(reg));
            if (it != regs.end()) {
                return current_index;
            }
        }

        auto write_regs_opt = get_insn_write_registers(insn);
        if (!write_regs_opt.has_value()) {
            std::cerr << "error: failed to obtain written registers" << std::endl;
            return {};
        }
        auto write_regs = write_regs_opt.value();

        for (auto reg : write_regs) {
            auto it = regs.find(static_cast<x86_reg>(reg));
            if (it != regs.end()) {
                return current_index;
            }
        }
    }
}

libexpr::expr_ptr resolver::create_constant_expr(libexpr::context& ctx, int64_t value) const {
    return ctx.make_expr<libexpr::constant_expr>(value);
}

libexpr::expr_ptr resolver::create_register_expr(libexpr::context& ctx, x86_reg reg) const {
    auto it = X86_64BITS_REGISTERS_NAME.find(reg);
    if (it == X86_64BITS_REGISTERS_NAME.end()) {
        std::cerr << "error: unknown x86 register name `" << reg << "`" << std::endl;
        return nullptr;
    }

    return ctx.make_expr<libexpr::register_expr>(it->second);
}

libexpr::expr_ptr resolver::create_operand_expr(libexpr::context& ctx, cs_insn& insn, cs_x86_op& operand) const {
    switch (operand.type) {
    case X86_OP_REG:
        return create_register_operand_expr(ctx, insn, operand.reg);
    case X86_OP_IMM:
        return create_immediate_operand_expr(ctx, operand.imm);
    case X86_OP_MEM:
        return create_memory_operand_expr(ctx, insn, operand.mem);
    default:
        std::cerr << "error: invalid X86 operand type" << std::endl;
        return nullptr;
    }
}

libexpr::expr_ptr resolver::create_register_operand_expr(libexpr::context& ctx, cs_insn& insn, x86_reg& reg) const {
    auto it_insn_ptr = X86_INSN_PTR_REGISTERS.find(reg);
    if (it_insn_ptr != X86_INSN_PTR_REGISTERS.end()) {
        return create_constant_expr(ctx, insn.address + insn.size);
    }

    auto it64 = X86_64BITS_REGISTERS.find(reg);
    if (it64 != X86_64BITS_REGISTERS.end()) {
        return create_register_expr(ctx, reg);
    }

    auto it32 = X86_32BITS_REGISTERS.find(reg);
    if (it32 != X86_32BITS_REGISTERS.end()) {
        return create_register_expr(ctx, it32->second);
    }

    auto it16 = X86_16BITS_REGISTERS.find(reg);
    if (it16 != X86_16BITS_REGISTERS.end()) {
        return create_register_expr(ctx, it16->second);
    }

    auto it8 = X86_8BITS_REGISTERS.find(reg);
    if (it8 != X86_8BITS_REGISTERS.end()) {
        return create_register_expr(ctx, it8->second);
    }

    std::cerr << "error: unsupported x86 register `" << reg << "`" << std::endl;
    return nullptr;
}

libexpr::expr_ptr resolver::create_immediate_operand_expr(libexpr::context& ctx, int64_t imm) const {
    return create_constant_expr(ctx, imm);
}

libexpr::expr_ptr resolver::create_memory_operand_expr(libexpr::context& ctx, cs_insn& insn, x86_op_mem& mem) const {
    if (mem.segment != X86_REG_INVALID) {
        std::cerr << "error: unsupported segment register `" << mem.segment << "`" << std::endl;
        return nullptr;
    }

    libexpr::expr_ptr mem_expr = nullptr;

    if (mem.index != X86_REG_INVALID) {
        if (mem.scale == 1) {
            auto index_expr = create_register_operand_expr(ctx, insn, mem.index);
            mem_expr        = index_expr;
        } else {
            auto index_expr = create_register_operand_expr(ctx, insn, mem.index);
            auto scale_expr = create_constant_expr(ctx, mem.scale);
            mem_expr        = create_expr<libexpr::mul_expr>(ctx, index_expr, scale_expr);
        }

        if (mem_expr == nullptr) {
            return nullptr;
        }
    }

    if (mem.base != X86_REG_INVALID) {
        auto base_expr = create_register_operand_expr(ctx, insn, mem.base);

        if (mem_expr != nullptr) {
            mem_expr = create_expr<libexpr::add_expr>(ctx, base_expr, mem_expr);
        } else {
            mem_expr = base_expr;
        }

        if (mem_expr == nullptr) {
            return nullptr;
        }
    }

    if (mem.disp != 0) {
        auto disp_expr = create_constant_expr(ctx, mem.disp);

        if (mem_expr != nullptr) {
            mem_expr = create_expr<libexpr::add_expr>(ctx, disp_expr, mem_expr);
        } else {
            mem_expr = disp_expr;
        }

        if (mem_expr == nullptr) {
            return nullptr;
        }
    }

    return mem_expr;
}

} /* namespace x86 */

} /* namespace librecover */

extern "C" {

int librecover_x86_resolve(csh handle, cs_insn* insns, size_t insn_count, size_t index, uint64_t** results,
                           void* memory) {
    std::vector<cs_insn*> insns_vec;
    for (size_t i = 0; i < insn_count; i++) {
        insns_vec.push_back(&insns[i]);
    }

    libexpr::process_memory process_memory;
    libexpr::memory_interface* memory_interface;
    if (memory != nullptr) {
        memory_interface = (libexpr::memory_interface*)memory;
    } else {
        memory_interface = &process_memory;
    }

    librecover::x86::resolver resolver(handle, insns_vec, *memory_interface, true);

    auto targets_opt = resolver.get_indirect_jump_targets(index);
    if (!targets_opt.has_value()) {
        *results = nullptr;
        return -1;
    }

    auto targets = targets_opt.value();

    if (targets.size() == 0) {
        *results = nullptr;
        return 0;
    }

    *results = (uint64_t*)malloc(targets.size()*sizeof(uint64_t));
    for (size_t i = 0; i < targets.size(); i++) {
        (*results)[i] = targets[i];
    }

    return targets.size();
}

} /* extern "C" */