#ifndef LIBEXPR_UTILS_HPP
#define LIBEXPR_UTILS_HPP

#include <algorithm>
#include <cstdint>
#include <string>

using int128_t  = __int128;
using uint128_t = unsigned __int128;

namespace nonstd {

static inline uint128_t make_u128(uint32_t w4, uint32_t w3, uint32_t w2, uint32_t w1) {
    uint128_t value = 0;
    value           = w4;
    value *= 0x10000;
    value *= 0x10000;
    value += w3;
    value *= 0x10000;
    value *= 0x10000;
    value += w2;
    value *= 0x10000;
    value *= 0x10000;
    value += w1;
    return value;
}

static inline std::string to_string(const int128_t& value) {
    bool is_negative = (value < 0);
    if (value == 0) {
        return "0";
    }

    std::string result = "";
    for (int128_t i = (is_negative ? -value : value); i > 0; i /= 10) {
        result.append(std::to_string(static_cast<int>(i % 10)));
    }

    if (is_negative) {
        result.append("-");
    }

    std::reverse(result.begin(), result.end());

    return result;
}

static inline std::string to_string_binary(const int128_t& value) {
    uint128_t copy = value;

    std::string result = "";
    for (int i = 0; i < 128; i++) {
        result += (copy & 1) ? '1' : '0';
        copy /= 2;
    }
    result += "b0";

    std::reverse(result.begin(), result.end());

    return result;
}

} /* namespace nonstd */

namespace utils {

template <typename T>
struct type_name {
    static std::string name;
};

} /* namespace utils */

#endif /* LIBEXPR_UTILS_HPP */