#include "libexpr/impl/process_memory.hpp"

template <typename OutputType>
static inline bool read_memory(uint64_t addr, OutputType& value) {
    OutputType* ptr = (OutputType*)addr;
    value           = *ptr;
    return true;
}

namespace libexpr {

bool process_memory::read(uint64_t addr, uint8_t& value) const {
    return read_memory(addr, value);
}

bool process_memory::read(uint64_t addr, uint16_t& value) const {
    return read_memory(addr, value);
}

bool process_memory::read(uint64_t addr, uint32_t& value) const {
    return read_memory(addr, value);
}

bool process_memory::read(uint64_t addr, uint64_t& value) const {
    return read_memory(addr, value);
}

bool process_memory::read(uint64_t addr, int8_t& value) const {
    return read_memory(addr, value);
}

bool process_memory::read(uint64_t addr, int16_t& value) const {
    return read_memory(addr, value);
}

bool process_memory::read(uint64_t addr, int32_t& value) const {
    return read_memory(addr, value);
}

bool process_memory::read(uint64_t addr, int64_t& value) const {
    return read_memory(addr, value);
}

} /* namespace libexpr */