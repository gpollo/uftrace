#include <unordered_map>

#include "libexpr/context.hpp"
#include "libexpr/utils.hpp"

namespace tests {

class dummy_memory : public libexpr::memory_interface {
public:
    bool debug_ = false;
    std::unordered_map<uint64_t, uint8_t> u8_;
    std::unordered_map<uint64_t, uint16_t> u16_;
    std::unordered_map<uint64_t, uint32_t> u32_;
    std::unordered_map<uint64_t, uint64_t> u64_;
    std::unordered_map<uint64_t, int8_t> i8_;
    std::unordered_map<uint64_t, int16_t> i16_;
    std::unordered_map<uint64_t, int32_t> i32_;
    std::unordered_map<uint64_t, int64_t> i64_;

    virtual ~dummy_memory() {}
    
    template <typename OutputType>
    bool read(uint64_t addr, OutputType& value, const std::unordered_map<uint64_t, OutputType>& mem) const {
        auto it = mem.find(addr);
        if (it == mem.end()) {
            return false;
        }

        value = it->second;
        return true;
    }

    template <typename OutputType>
    bool read_debug(uint64_t addr, OutputType& value, const std::unordered_map<uint64_t, OutputType>& mem) const {
        bool success = read(addr, value, mem);
        if (!debug_) {
            return success;
        }

        auto type = utils::type_name<OutputType>::name;
        std::cerr << "read (" << type << ") "<< "at address 0x" << std::hex << addr << " = ";

        if (success) {
            std::cerr << std::dec << value << std::endl;
        } else {
            std::cerr << "failed" << std::endl;
        }

        return success;
    }

    virtual bool read(uint64_t addr, uint8_t& value) const {
        return read_debug(addr, value, u8_);
    }

    virtual bool read(uint64_t addr, uint16_t& value) const {
        return read_debug(addr, value, u16_);
    }

    virtual bool read(uint64_t addr, uint32_t& value) const {
        return read_debug(addr, value, u32_);
    }

    virtual bool read(uint64_t addr, uint64_t& value) const {
        return read_debug(addr, value, u64_);
    }

    virtual bool read(uint64_t addr, int8_t& value) const {
        return read_debug(addr, value, i8_);
    }

    virtual bool read(uint64_t addr, int16_t& value) const {
        return read_debug(addr, value, i16_);
    }

    virtual bool read(uint64_t addr, int32_t& value) const {
        return read_debug(addr, value, i32_);
    }

    virtual bool read(uint64_t addr, int64_t& value) const {
        return read_debug(addr, value, i64_);
    }
};

} /* namespace tests */