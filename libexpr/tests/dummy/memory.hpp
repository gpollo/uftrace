#include "libexpr/context.hpp"

namespace tests {

/* TODO: replace usages by mock classes */

class dummy_memory : public libexpr::memory_interface {
public:
    virtual ~dummy_memory() {}
    
    virtual bool read(uint64_t addr, uint8_t& value) const {
        return true;
    }

    virtual bool read(uint64_t addr, uint16_t& value) const {
        return true;
    }

    virtual bool read(uint64_t addr, uint32_t& value) const {
        return true;
    }

    virtual bool read(uint64_t addr, uint64_t& value) const {
        return true;
    }

    virtual bool read(uint64_t addr, int8_t& value) const {
        return true;
    }

    virtual bool read(uint64_t addr, int16_t& value) const {
        return true;
    }

    virtual bool read(uint64_t addr, int32_t& value) const {
        return true;
    }

    virtual bool read(uint64_t addr, int64_t& value) const {
        return true;
    }
};

} /* namespace tests */