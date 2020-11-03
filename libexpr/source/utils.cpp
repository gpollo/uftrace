#include "libexpr/utils.hpp"

namespace utils {

template <>
std::string type_name<uint8_t>::name = "uint8_t";

template <>
std::string type_name<uint16_t>::name = "uint16_t";

template <>
std::string type_name<uint32_t>::name = "uint32_t";

template <>
std::string type_name<uint64_t>::name = "uint64_t";

template <>
std::string type_name<int8_t>::name = "int8_t";

template <>
std::string type_name<int16_t>::name = "int16_t";

template <>
std::string type_name<int32_t>::name = "int32_t";

template <>
std::string type_name<int64_t>::name = "int64_t";

} /* namespace utils */