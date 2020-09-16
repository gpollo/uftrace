#include "catch.hpp"
#include "dummy/memory.hpp"
#include "libexpr/expr/bound.hpp"
#include "libexpr/expr/constant.hpp"
#include "libexpr/expr/extend.hpp"
#include "libexpr/expr/memory.hpp"
#include "libexpr/expr/operation.hpp"
#include "libexpr/expr/register.hpp"
#include "libexpr/expr/root.hpp"

TEST_CASE("expression creation works", "[libexpr::context]") {
    tests::dummy_memory memory;
    libexpr::context context(memory);

    SECTION("parent and child should be ok") {
        auto constant = context.make_expr<libexpr::constant_expr>(100);
        auto root     = context.make_expr<libexpr::root_expr>(constant);

        REQUIRE(constant->get_parent() == root);
        REQUIRE(root->get_children().size() == 1);
        REQUIRE(root->get_children()[0] == constant);
    }
}

TEST_CASE("root insertion works", "[libexpr::context]") {
    tests::dummy_memory memory;
    libexpr::context context(memory);

    SECTION("adding root node should set it") {
        auto constant = context.make_expr<libexpr::constant_expr>(100);
        REQUIRE(context.get_root_expr() == nullptr);

        auto root = context.make_expr<libexpr::root_expr>(constant);
        REQUIRE(context.get_root_expr() == nullptr);

        REQUIRE(context.add_expr(root));
        REQUIRE(context.get_root_expr() == root);
    }

    SECTION("adding non-root should not set it") {
        auto constant = context.make_expr<libexpr::constant_expr>(100);
        REQUIRE(context.get_root_expr() == nullptr);

        REQUIRE(!context.add_expr(constant));
        REQUIRE(context.get_root_expr() == nullptr);
    }
}

std::optional<libexpr::range> get_possible_values(libexpr::expr_ptr expr) {
    return expr->get_possible_values();
}

TEST_CASE("resolving expression works", "[libexpr::context]") {
    tests::dummy_memory memory;
    libexpr::context context(memory);

    SECTION("test 1") {
        /*
         * 0xc0:   pushq   %rbp
         * 0xc1:   movq    %rsp, %rbp
         * 0xc4:   movl    %edi, -4(%rbp)
         * 0xc7:   cmpl    $6, -4(%rbp)
         * 0xcb:   ja      0x112
         * 0xcd:   movl    -4(%rbp), %eax
         * 0xd0:   leaq    (, %rax, 4), %rdx
         * 0xd8:   leaq    0x3c2bd(%rip), %rax
         * 0xdf:   movl    (%rdx, %rax), %eax
         * 0xe2:   cltq
         * 0xe4:   leaq    0x3c2b1(%rip), %rdx
         * 0xeb:   addq    %rdx, %rax
         * 0xee:   jmpq    *%rax
         */
        unsigned int current_rip = 0;

        /* jmpq *%rax */
        auto rax  = context.make_expr<libexpr::register_expr>("rax");
        auto root = context.make_expr<libexpr::root_expr>(rax);
        REQUIRE(context.add_expr(root));
        REQUIRE(context.is_there_any_unbounded_register());
        REQUIRE(context.get_expr_count() == 2);
        REQUIRE(rax->get_parent() == root);
        REQUIRE(root->get_parent() == nullptr);

        auto rax_values = get_possible_values(rax);
        REQUIRE(rax_values.has_value());
        REQUIRE(rax_values.value() == libexpr::range());

        /* addq %rdx, %rax */
        auto rep1_rdx    = context.make_expr<libexpr::register_expr>("rdx");
        auto rep1_rax    = context.make_expr<libexpr::register_expr>("rax");
        auto rep1_add    = context.make_expr<libexpr::add_expr>(rep1_rax, rep1_rdx);
        auto target1_rax = context.make_expr<libexpr::register_expr>("rax");
        REQUIRE(context.resolve_expr(target1_rax, rep1_add) == 1);
        REQUIRE(context.is_there_any_unbounded_register());
        REQUIRE(context.get_expr_count() == 4);
        REQUIRE(rep1_rdx->get_parent() == rep1_add);
        REQUIRE(rep1_rax->get_parent() == rep1_add);

        auto rep1_rdx_values = get_possible_values(rep1_rdx);
        REQUIRE(!rep1_rdx_values.has_value());

        auto rep1_rax_values = get_possible_values(rep1_rax);
        REQUIRE(!rep1_rax_values.has_value());

        /* leaq 0x3c2b1(%rip), %rdx */
        auto rep2_constant = context.make_expr<libexpr::constant_expr>(0x3c2b1 + current_rip);
        auto target2_rdx   = context.make_expr<libexpr::register_expr>("rdx");
        REQUIRE(context.resolve_expr(target2_rdx, rep2_constant) == 1);
        REQUIRE(context.is_there_any_unbounded_register());
        REQUIRE(context.get_expr_count() == 4);
        REQUIRE(rep2_constant->get_parent() == rep1_add);

        auto rep2_constant_values = get_possible_values(rep2_constant);
        REQUIRE(!rep2_constant_values.has_value());

        rep1_rax_values = get_possible_values(rep1_rax);
        REQUIRE(rep1_rax_values.has_value());
        REQUIRE(rep1_rax_values.value() == libexpr::range());

        /* cltq */
        auto rep3_eax    = context.make_expr<libexpr::register_expr>("rax");
        auto rep3_extend = context.make_expr<libexpr::sign_extend32_expr>(rep3_eax);
        auto target3_rax = context.make_expr<libexpr::register_expr>("rax");
        REQUIRE(context.resolve_expr(target3_rax, rep3_extend) == 1);
        REQUIRE(context.is_there_any_unbounded_register());
        REQUIRE(context.get_expr_count() == 5);
        REQUIRE(rep3_eax->get_parent() == rep3_extend);
        REQUIRE(rep3_extend->get_parent() == rep1_add);

        auto rep3_eax_constant_values = get_possible_values(rep3_eax);
        REQUIRE(rep3_eax_constant_values.has_value());
        REQUIRE(rep3_eax_constant_values.value() == libexpr::range());

        /* movl (%rdx, %rax), %eax */
        auto rep4_rax    = context.make_expr<libexpr::register_expr>("rax");
        auto rep4_rdx    = context.make_expr<libexpr::register_expr>("rdx");
        auto rep4_add    = context.make_expr<libexpr::add_expr>(rep4_rax, rep4_rdx);
        auto rep4_memory = context.make_expr<libexpr::memory32_expr<uint64_t>>(rep4_add);
        auto target4_eax = context.make_expr<libexpr::register_expr>("rax");
        REQUIRE(context.resolve_expr(target4_eax, rep4_memory) == 1);
        REQUIRE(context.is_there_any_unbounded_register());
        REQUIRE(context.get_expr_count() == 8);
        REQUIRE(rep4_rax->get_parent() == rep4_add);
        REQUIRE(rep4_rdx->get_parent() == rep4_add);
        REQUIRE(rep4_add->get_parent() == rep4_memory);
        REQUIRE(rep4_memory->get_parent() == rep3_extend);

        auto rep4_rax_values = get_possible_values(rep4_rax);
        REQUIRE(!rep4_rax_values.has_value());

        auto rep4_rdx_values = get_possible_values(rep4_rdx);
        REQUIRE(!rep4_rdx_values.has_value());

        /* leaq 0x3c2bd(%rip), %rax */
        auto rep5_constant = context.make_expr<libexpr::constant_expr>(0x3c2bd + current_rip);
        auto target5_rax   = context.make_expr<libexpr::register_expr>("rax");
        REQUIRE(context.resolve_expr(target5_rax, rep5_constant) == 1);
        REQUIRE(context.is_there_any_unbounded_register());
        REQUIRE(context.get_expr_count() == 8);
        REQUIRE(rep5_constant->get_parent() == rep4_add);

        auto rep5_constant_values = get_possible_values(rep5_constant);
        REQUIRE(!rep5_constant_values.has_value());

        rep4_rdx_values = get_possible_values(rep4_rdx);
        REQUIRE(rep4_rdx_values.has_value());
        REQUIRE(rep4_rdx_values.value() == libexpr::range());

        /* leaq (, %rax, 4), %rdx */
        auto rep6_constant = context.make_expr<libexpr::constant_expr>(4);
        auto rep6_rax      = context.make_expr<libexpr::register_expr>("rax");
        auto rep6_mul      = context.make_expr<libexpr::mul_expr>(rep6_rax, rep6_constant);
        auto target6_rdx   = context.make_expr<libexpr::register_expr>("rdx");
        REQUIRE(context.resolve_expr(target6_rdx, rep6_mul) == 1);
        REQUIRE(context.is_there_any_unbounded_register());
        REQUIRE(context.get_expr_count() == 10);
        REQUIRE(rep6_constant->get_parent() == rep6_mul);
        REQUIRE(rep6_rax->get_parent() == rep6_mul);
        REQUIRE(rep6_mul->get_parent() == rep4_add);

        auto rep6_constant_values = get_possible_values(rep6_constant);
        REQUIRE(!rep6_constant_values.has_value());

        auto rep6_rax_values = get_possible_values(rep6_rax);
        REQUIRE(rep6_rax_values.has_value());
        REQUIRE(rep6_rax_values.value() == libexpr::range());

        /* movl -4(%rbp), %eax */
        auto rep7_constant = context.make_expr<libexpr::constant_expr>(-4);
        auto rep7_rbp      = context.make_expr<libexpr::register_expr>("rbp");
        auto rep7_add      = context.make_expr<libexpr::add_expr>(rep7_rbp, rep7_constant);
        auto rep7_extend   = context.make_expr<libexpr::zero_extend32_expr>(rep7_add);
        auto target7_eax   = context.make_expr<libexpr::register_expr>("rax");
        REQUIRE(context.resolve_expr(target7_eax, rep7_extend) == 1);
        REQUIRE(context.is_there_any_unbounded_register());
        REQUIRE(context.get_expr_count() == 13);
        REQUIRE(rep7_constant->get_parent() == rep7_add);
        REQUIRE(rep7_rbp->get_parent() == rep7_add);
        REQUIRE(rep7_add->get_parent() == rep7_extend);
        REQUIRE(rep7_extend->get_parent() == rep6_mul);

        auto rep7_constant_values = get_possible_values(rep7_constant);
        REQUIRE(!rep7_constant_values.has_value());

        auto rep7_rbp_values = get_possible_values(rep7_rbp);
        REQUIRE(rep7_rbp_values.has_value());
        REQUIRE(rep7_rbp_values.value() == libexpr::range());

        /* cmpl $6, -4(%rbp) */
        /* ja 0x112 */
        auto rep8_constant       = context.make_expr<libexpr::constant_expr>(-4);
        auto rep8_rbp            = context.make_expr<libexpr::register_expr>("rbp");
        auto rep8_add            = context.make_expr<libexpr::add_expr>(rep8_rbp, rep8_constant);
        auto rep8_constant_limit = context.make_expr<libexpr::constant_expr>(6);
        auto rep8_upper_bound    = context.make_expr<libexpr::upper_bound_expr>(rep8_add, rep8_constant_limit);
        auto target8_constant    = context.make_expr<libexpr::constant_expr>(-4);
        auto target8_rbp         = context.make_expr<libexpr::register_expr>("rbp");
        auto target8_add         = context.make_expr<libexpr::add_expr>(target8_rbp, target8_constant);
        REQUIRE(context.resolve_expr(target8_add, rep8_upper_bound) == 1);
        REQUIRE(!context.is_there_any_unbounded_register());
        REQUIRE(context.get_expr_count() == 15);
        REQUIRE(rep8_constant->get_parent() == rep8_add);
        REQUIRE(rep8_rbp->get_parent() == rep8_add);
        REQUIRE(rep8_add->get_parent() == rep8_upper_bound);
        REQUIRE(rep8_constant_limit->get_parent() == rep8_upper_bound);
        REQUIRE(rep8_upper_bound->get_parent() == rep7_extend);

        auto rep8_constant_values = get_possible_values(rep8_constant);
        REQUIRE(!rep8_constant_values.has_value());

        auto rep8_rbp_values = get_possible_values(rep8_rbp);
        REQUIRE(rep8_rbp_values.has_value());
        REQUIRE(rep8_rbp_values.value() == libexpr::range(libexpr::range::infinity::value, 10));
    }

    SECTION("test 2") {
        /*
         * 0x73:   pushq   %rbp
         * 0x74:   movq    %rsp, %rbp
         * 0x77:   movl    %edi, -4(%rbp)
         * 0x7a:   movl    -4(%rbp), %eax
         * 0x7d:   addl    $1, %eax
         * 0x80:   cmpl    $6, %eax
         * 0x83:   ja      0xdd
         * 0x85:   movl    %eax, %eax
         * 0x87:   leaq    (, %rax, 4), %rdx
         * 0x8f:   leaq    0x3886c(%rip), %rax
         * 0x96:   movl    (%rdx, %rax), %eax
         * 0x99:   cltq
         * 0x9b:   leaq    0x38860(%rip), %rdx
         * 0xa2:   addq    %rdx, %rax
         * 0xa5:   jmpq    *%rax
         */
        unsigned int current_rip = 0;

        /* jmpq *%rax */
        auto rax  = context.make_expr<libexpr::register_expr>("rax");
        auto root = context.make_expr<libexpr::root_expr>(rax);
        REQUIRE(context.add_expr(root));
        REQUIRE(context.is_there_any_unbounded_register());
        REQUIRE(context.get_expr_count() == 2);
        REQUIRE(rax->get_parent() == root);
        REQUIRE(root->get_parent() == nullptr);

        auto rax_values = get_possible_values(rax);
        REQUIRE(rax_values.has_value());
        REQUIRE(rax_values.value() == libexpr::range());

        /* addq %rdx, %rax */
        auto rep1_rdx    = context.make_expr<libexpr::register_expr>("rdx");
        auto rep1_rax    = context.make_expr<libexpr::register_expr>("rax");
        auto rep1_add    = context.make_expr<libexpr::add_expr>(rep1_rax, rep1_rdx);
        auto target1_rax = context.make_expr<libexpr::register_expr>("rax");
        REQUIRE(context.resolve_expr(target1_rax, rep1_add) == 1);
        REQUIRE(context.is_there_any_unbounded_register());
        REQUIRE(context.get_expr_count() == 4);
        REQUIRE(rep1_rdx->get_parent() == rep1_add);
        REQUIRE(rep1_rax->get_parent() == rep1_add);

        auto rep1_rdx_values = get_possible_values(rep1_rdx);
        REQUIRE(!rep1_rdx_values.has_value());

        auto rep1_rax_values = get_possible_values(rep1_rax);
        REQUIRE(!rep1_rax_values.has_value());

        /* leaq 0x38860(%rip), %rdx */
        auto rep2_constant = context.make_expr<libexpr::constant_expr>(0x38860 + current_rip);
        auto target2_rdx   = context.make_expr<libexpr::register_expr>("rdx");
        REQUIRE(context.resolve_expr(target2_rdx, rep2_constant) == 1);
        REQUIRE(context.is_there_any_unbounded_register());
        REQUIRE(context.get_expr_count() == 4);
        REQUIRE(rep2_constant->get_parent() == rep1_add);

        auto rep2_constant_values = get_possible_values(rep2_constant);
        REQUIRE(!rep2_constant_values.has_value());

        rep1_rax_values = get_possible_values(rep1_rax);
        REQUIRE(rep1_rax_values.has_value());
        REQUIRE(rep1_rax_values.value() == libexpr::range());

        /* cltq */
        auto rep3_eax    = context.make_expr<libexpr::register_expr>("rax");
        auto rep3_extend = context.make_expr<libexpr::sign_extend32_expr>(rep3_eax);
        auto target3_rax = context.make_expr<libexpr::register_expr>("rax");
        REQUIRE(context.resolve_expr(target3_rax, rep3_extend) == 1);
        REQUIRE(context.is_there_any_unbounded_register());
        REQUIRE(context.get_expr_count() == 5);
        REQUIRE(rep3_eax->get_parent() == rep3_extend);
        REQUIRE(rep3_extend->get_parent() == rep1_add);

        auto rep3_eax_constant_values = get_possible_values(rep3_eax);
        REQUIRE(rep3_eax_constant_values.has_value());
        REQUIRE(rep3_eax_constant_values.value() == libexpr::range());

        /* movl (%rdx, %rax), %eax */
        auto rep4_rax    = context.make_expr<libexpr::register_expr>("rax");
        auto rep4_rdx    = context.make_expr<libexpr::register_expr>("rdx");
        auto rep4_add    = context.make_expr<libexpr::add_expr>(rep4_rax, rep4_rdx);
        auto rep4_memory = context.make_expr<libexpr::memory32_expr<uint64_t>>(rep4_add);
        auto target4_eax = context.make_expr<libexpr::register_expr>("rax");
        REQUIRE(context.resolve_expr(target4_eax, rep4_memory) == 1);
        REQUIRE(context.is_there_any_unbounded_register());
        REQUIRE(context.get_expr_count() == 8);
        REQUIRE(rep4_rax->get_parent() == rep4_add);
        REQUIRE(rep4_rdx->get_parent() == rep4_add);
        REQUIRE(rep4_add->get_parent() == rep4_memory);
        REQUIRE(rep4_memory->get_parent() == rep3_extend);

        auto rep4_rax_values = get_possible_values(rep4_rax);
        REQUIRE(!rep4_rax_values.has_value());

        auto rep4_rdx_values = get_possible_values(rep4_rdx);
        REQUIRE(!rep4_rdx_values.has_value());

        // /* leaq 0x3886c(%rip), %rax */
        auto rep5_constant = context.make_expr<libexpr::constant_expr>(0x3886c + current_rip);
        auto target5_rax   = context.make_expr<libexpr::register_expr>("rax");
        REQUIRE(context.resolve_expr(target5_rax, rep5_constant) == 1);
        REQUIRE(context.is_there_any_unbounded_register());
        REQUIRE(context.get_expr_count() == 8);
        REQUIRE(rep5_constant->get_parent() == rep4_add);

        auto rep5_constant_values = get_possible_values(rep5_constant);
        REQUIRE(!rep5_constant_values.has_value());

        rep4_rdx_values = get_possible_values(rep4_rdx);
        REQUIRE(rep4_rdx_values.has_value());
        REQUIRE(rep4_rdx_values.value() == libexpr::range());

        /* leaq (, %rax, 4), %rdx */
        auto rep6_constant = context.make_expr<libexpr::constant_expr>(4);
        auto rep6_rax      = context.make_expr<libexpr::register_expr>("rax");
        auto rep6_mul      = context.make_expr<libexpr::mul_expr>(rep6_rax, rep6_constant);
        auto target6_rdx   = context.make_expr<libexpr::register_expr>("rdx");
        REQUIRE(context.resolve_expr(target6_rdx, rep6_mul) == 1);
        REQUIRE(context.is_there_any_unbounded_register());
        REQUIRE(context.get_expr_count() == 10);
        REQUIRE(rep6_constant->get_parent() == rep6_mul);
        REQUIRE(rep6_rax->get_parent() == rep6_mul);
        REQUIRE(rep6_mul->get_parent() == rep4_add);

        auto rep6_constant_values = get_possible_values(rep6_constant);
        REQUIRE(!rep6_constant_values.has_value());

        auto rep6_rax_values = get_possible_values(rep6_rax);
        REQUIRE(rep6_rax_values.has_value());
        REQUIRE(rep6_rax_values.value() == libexpr::range());

        /* movl %eax, %eax */
        auto rep7_eax    = context.make_expr<libexpr::register_expr>("rax");
        auto rep7_extend = context.make_expr<libexpr::zero_extend32_expr>(rep7_eax);
        auto target7_eax = context.make_expr<libexpr::register_expr>("rax");
        REQUIRE(context.resolve_expr(target7_eax, rep7_extend) == 1);
        REQUIRE(context.is_there_any_unbounded_register());
        REQUIRE(context.get_expr_count() == 11);
        REQUIRE(rep7_eax->get_parent() == rep7_extend);
        REQUIRE(rep7_extend->get_parent() == rep6_mul);

        auto rep7_eax_values = get_possible_values(rep7_eax);
        REQUIRE(rep7_eax_values.has_value());
        REQUIRE(rep7_eax_values.value() == libexpr::range());

        auto rep7_extend_values = get_possible_values(rep7_extend);
        REQUIRE(rep7_extend_values.has_value());
        REQUIRE(rep7_extend_values.value() == libexpr::range());

        /* cmpl $6, %eax */
        /* ja 0xdd */
        auto rep8_eax            = context.make_expr<libexpr::register_expr>("rax");
        auto rep8_constant_limit = context.make_expr<libexpr::constant_expr>(6);
        auto rep8_upper_bound    = context.make_expr<libexpr::upper_bound_expr>(rep8_eax, rep8_constant_limit);
        auto target8_eax         = context.make_expr<libexpr::register_expr>("rax");
        REQUIRE(context.resolve_expr(target8_eax, rep8_upper_bound) == 1);
        REQUIRE(!context.is_there_any_unbounded_register());
        REQUIRE(context.get_expr_count() == 13);
        REQUIRE(rep8_eax->get_parent() == rep8_upper_bound);
        REQUIRE(rep8_constant_limit->get_parent() == rep8_upper_bound);
        REQUIRE(rep8_upper_bound->get_parent() == rep7_extend);

        auto rep8_eax_values = get_possible_values(rep8_eax);
        REQUIRE(rep8_eax_values.has_value());
        REQUIRE(rep8_eax_values.value() == libexpr::range(libexpr::range::infinity::value, 6));
    }
}