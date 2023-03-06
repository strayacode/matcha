#include <filesystem>
#include "common/filesystem.h"

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
  
} // namespace common