#pragma once

#include <array>
#include "common/types.h"
#include "common/log.h"

namespace common {

struct VirtualPageTable {
    void Reset() {
        page_table.fill(nullptr);
    }

    void Map(u8* data, VirtualAddress base, u32 size, u32 mask) {
        for (u32 offset = 0; offset < size; offset += PAGE_SIZE) {
            VirtualAddress vaddr = base + offset;
            int index = vaddr >> PAGE_BITS;
            page_table[index] = &data[vaddr & mask];
        }
    }

    template <typename T>
    T* Lookup(VirtualAddress vaddr) {
        int index = vaddr >> PAGE_BITS;
        if (page_table[index] == nullptr) {
            return nullptr;
        }

        int offset = vaddr & PAGE_MASK;
        return reinterpret_cast<T*>(page_table[index] + offset);
    }

private:
    static constexpr int PAGE_BITS = 12;
    static constexpr int PAGE_SIZE = 1 << PAGE_BITS;
    static constexpr u32 PAGE_MASK = PAGE_SIZE - 1;
    static constexpr int PAGE_COUNT = 1 << (32 - PAGE_BITS);

    std::array<u8*, PAGE_COUNT> page_table;
};

} // namespace common