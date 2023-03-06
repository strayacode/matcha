#include <algorithm>
#include <cctype>
#include "common/string.h"

namespace common {

std::string ToLower(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), [](char c) {
        return std::tolower(c);
    });

    return str;
}

std::string ToUpper(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), [](char c) {
        return std::toupper(c);
    });

    return str;
}

} // namespace common