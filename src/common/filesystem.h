#pragma once

#include <string>
#include <vector>

namespace common {

std::vector<std::string> ScanDirectoryRecursive(const std::string& path, std::vector<std::string> extensions);

} // namespace common