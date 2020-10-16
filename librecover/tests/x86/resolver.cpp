#include "librecover/x86/resolver.hpp"
#include "catch.hpp"
#include "dummy/memory.hpp"
#include "librecover/utils.hpp"
#include "librecover/x86/resolver.h"

TEST_CASE("resolving indirect jumps works", "[librecover::x86::resolver]") {
    SECTION("test 1") {
        const unsigned char* bytes = (unsigned char*)
            "\x83\xff\x14"                 /* 0 cmp    $0x14,%edi */
            "\x48\x89\xf0"                 /* 1 mov    %rsi,%rax */
            "\x77\x18"                     /* 2 ja     1320 */
            "\x48\x8d\x0d\x11\x0d\x00\x00" /* 3 lea    0xd11(%rip),%rcx */
            "\x89\xff"                     /* 4 mov    %edi,%edi */
            "\x48\x63\x14\xb9"             /* 5 movslq (%rcx,%rdi,4),%rdx */
            "\x48\x01\xca"                 /* 6 add    %rcx,%rdx */
            "\xff\xe2";                    /* 7 jmpq   *%rdx */
        const size_t bytes_len = 26;

        csh handle;
        REQUIRE(cs_open(CS_ARCH_X86, CS_MODE_64, &handle) == CS_ERR_OK);
        REQUIRE(cs_option(handle, CS_OPT_DETAIL, CS_OPT_ON) == CS_ERR_OK);

        cs_insn* insns;
        const size_t insn_count = 8;
        REQUIRE(cs_disasm(handle, bytes, bytes_len, 2000, 0, &insns) == insn_count);

        tests::dummy_memory memory;
        memory.i32_ = {
            {0x14f0, 10}, {0x14f4, 11}, {0x14f8, 12}, {0x14fc, 13}, {0x1500, 14}, {0x1504, 15}, {0x1508, 16},
            {0x150c, 17}, {0x1510, 18}, {0x1514, 19}, {0x1518, 20}, {0x151c, 21}, {0x1520, 22}, {0x1524, 23},
            {0x1528, 24}, {0x152c, 25}, {0x1530, 26}, {0x1534, 27}, {0x1538, 28}, {0x153c, 29}, {0x1540, 30},
        };

        std::unordered_set<uint64_t> expected{
            5370, 5371, 5372, 5373, 5374, 5375, 5376, 5377, 5378, 5379, 5380,
            5381, 5382, 5383, 5384, 5385, 5386, 5387, 5388, 5389, 5390,
        };

        uint64_t* results;
        int result_count = librecover_x86_resolve(handle, insns, insn_count, insn_count - 1, &results, &memory);

        REQUIRE(result_count == (int)expected.size());
        for (int i = 0; i < result_count; i++) {
            REQUIRE(expected.find(results[i]) != expected.end());
        }

        free(results);
        cs_free(insns, insn_count);
        cs_close(&handle);
    }

    SECTION("test 2") {
        const unsigned char* bytes = (unsigned char*)
            "\x83\x7d\xfc\x06"                 /* cmpl  $0x6,-0x4(%rbp) */
            "\x77\x2b"                         /* ja    53337 */
            "\x8b\x45\xfc"                     /* mov  -0x4(%rbp),%eax */
            "\x48\x8d\x14\x85\x00\x00\x00\x00" /* lea   0x0(,%rax,4),%rdx */
            "\x48\x8d\x05\x7a\x7e\x01\x00"     /* lea   0x17e7a(%rip),%rax */
            "\x8b\x04\x02"                     /* mov   (%rdx,%rax,1),%eax */
            "\x48\x98"                         /* cltq */
            "\x48\x8d\x15\x6e\x7e\x01\x00"     /* lea   0x17e6e(%rip),%rdx */
            "\x48\x01\xd0"                     /* add   %rdx,%rax */
            "\xff\xe0";                        /* jmpq  *%rax */
        const size_t bytes_len = 41;

        csh handle;
        REQUIRE(cs_open(CS_ARCH_X86, CS_MODE_64, &handle) == CS_ERR_OK);
        REQUIRE(cs_option(handle, CS_OPT_DETAIL, CS_OPT_ON) == CS_ERR_OK);

        cs_insn* insns;
        const size_t insn_count = 10;
        REQUIRE(cs_disasm(handle, bytes, bytes_len, 2000, 0, &insns) == insn_count);

        tests::dummy_memory memory;
        memory.i32_ = {
            {0x18662, 40}, {0x18666, 41}, {0x1866a, 42}, {0x1866e, 43}, {0x18672, 44}, {0x18676, 45}, {0x1867a, 46},
        };

        std::unordered_set<uint64_t> expected{
            99978, 99979, 99980, 99981, 99982, 99983, 99984,
        };

        uint64_t* results;
        int result_count = librecover_x86_resolve(handle, insns, insn_count, insn_count - 1, &results, &memory);

        REQUIRE(result_count == (int)expected.size());
        for (int i = 0; i < result_count; i++) {
            REQUIRE(expected.find(results[i]) != expected.end());
        }

        free(results);
        cs_free(insns, insn_count);
        cs_close(&handle);
    }
}