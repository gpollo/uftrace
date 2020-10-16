#ifndef LIBRECOVER_INCLUDE_X86_RESOLVER_H
#define LIBRECOVER_INCLUDE_X86_RESOLVER_H

#include <stdint.h>

#include "capstone/capstone.h"

#ifdef __cplusplus
extern "C" {
#endif

int librecover_x86_resolve(csh handle, cs_insn* insns, size_t insn_count, size_t index, uint64_t** results, void* memory);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LIBRECOVER_INCLUDE_X86_RESOLVER_H */