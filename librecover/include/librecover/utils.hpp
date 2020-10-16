#ifndef LIBRECOVER_INCLUDE_UTILS_HPP
#define LIBRECOVER_INCLUDE_UTILS_HPP

#include <iostream>
#include <sstream>
#include <string>

namespace librecover {

namespace utils {

template <typename T>
std::string debug(const T& obj) {
    std::ostringstream out;
    out << obj;

    std::istringstream in(out.str());
    std::string line;

    std::ostringstream string;
    while (std::getline(in, line)) {
        string << "debug: " << line << std::endl;
    }

    return string.str();
}

} /* namespace utils */

} /* namespace librecover */

#endif /* LIBRECOVER_INCLUDE_UTILS_HPP */