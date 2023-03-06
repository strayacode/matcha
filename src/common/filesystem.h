#pragma once

#include <string>
#include <vector>
#include "common/types.h"

namespace common {

std::vector<std::string> ScanDirectoryRecursive(const std::string& path, std::vector<std::string> extensions);

std::string GetFormattedSize(u64 size);

} // namespace common