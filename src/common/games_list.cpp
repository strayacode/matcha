#include <filesystem>
#include <algorithm>
#include "common/games_list.h"
#include "common/filesystem.h"
#include "common/string.h"

namespace common {

void GamesList::Initialise() {
    std::vector<std::string> extensions = {".elf", ".iso", ".irx"};
    std::vector<std::string> filepaths = ScanDirectoryRecursive("../roms", extensions);
    for (const std::string& path : filepaths) {
        AddEntry(path);
    }

    std::sort(entries.begin(), entries.end(), [](Entry a, Entry b) {
        return common::ToLower(a.name).compare(common::ToLower(b.name)) < 0;
    });
}

void GamesList::AddEntry(const std::string& path) {
    Entry entry;
    entry.path = path;
    entry.name = std::filesystem::path(path).stem();
    entries.push_back(entry);
}

} // namespace common