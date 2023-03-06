
#pragma once

#include <string>
#include <vector>

namespace common {

class GamesList {
public:
    void Initialise();

    struct Entry {
        std::string path;
        std::string name;
    };

    using Entries = std::vector<Entry>;

    Entries& GetEntries() { return entries; }

private:
    void AddEntry(const std::string& path);

    Entries entries;
};

} // namespace common