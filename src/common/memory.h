#pragma once

#include <cstring>
#include "common/types.h"

namespace common {

template <typename T>
inline T Read(void* data, int offset = 0) {
    T return_value = 0;
    std::memcpy(&return_value, (u8*)data + offset, sizeof(T));
    return return_value;
}

template <typename T>
inline void Write(void* data, T value, int offset = 0) {
    std::memcpy((u8*)data + offset, &value, sizeof(T));
}

inline bool InRange(u32 start, u32 end, u32 addr) {
    return addr >= start && addr < end;
}

} // namespace common