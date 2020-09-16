#ifndef LIBEXPR_IMPL_PROCESS_MEMORY_HPP
#define LIBEXPR_IMPL_PROCESS_MEMORY_HPP

#include "libexpr/context.hpp"

namespace libexpr {

class process_memory : public memory_interface {
   public:
    virtual ~process_memory() = default;

    virtual bool read(uint64_t addr, uint8_t& value) const;
    virtual bool read(uint64_t addr, uint16_t& value) const;
    virtual bool read(uint64_t addr, uint32_t& value) const;
    virtual bool read(uint64_t addr, uint64_t& value) const;

    virtual bool read(uint64_t addr, int8_t& value) const;
    virtual bool read(uint64_t addr, int16_t& value) const;
    virtual bool read(uint64_t addr, int32_t& value) const;
    virtual bool read(uint64_t addr, int64_t& value) const;
};

} /* namespace libexpr */

#endif /* LIBEXPR_IMPL_PROCESS_MEMORY_HPP */
