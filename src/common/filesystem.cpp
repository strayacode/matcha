#include <filesystem>
#include <cmath>
#include "common/filesystem.h"
#include "common/string.h"

namespace common {

std::vector<std::string> ScanDirectoryRecursive(const std::string& path, std::vector<std::string> extensions) {
    std::vector<std::string> files;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(path)) {
        if (!std::filesystem::is_directory(entry)) {
            const std::string& extension = std::filesystem::path(entry).extension();
            if (std::find(extensions.begin(), extensions.end(), extension) != extensions.end()) {
                files.push_back(entry.path());
            }
        }
    }
    return files;
}

std::string GetFormattedSize(u64 size) {
    int i = 0;
    double mantissa = size;
    static const char* size_types = "BKMGTPE";

    while (mantissa >= 1024) {
        mantissa /= 1024;
        i++;
    }

    mantissa = std::ceil(mantissa * 10.0f) / 10.0f;
    std::string mantissa_str = common::Format("%.2f", mantissa);

    if (mantissa_str.back() == '0') {
        mantissa_str = mantissa_str.substr(0, mantissa_str.size() - 1);
    }

    if (mantissa_str.back() == '0') {
        mantissa_str = mantissa_str.substr(0, mantissa_str.size() - 2);
    }

    if (i > 0) {
        return common::Format("%s %cB", mantissa_str.c_str(), size_types[i]);
    }

    return common::Format("%s B", mantissa_str.c_str());
}
  
} // namespace common