#pragma once

#include "common/types.h"
#include "common/memory.h"

namespace common {

template <typename T, int size>
T SignExtend(T value) {
    struct {
        T data : size;
    } s;

    s.data = value;
    return s.data;
}

template <typename To, typename From>
inline To BitCast(From& value) {
    static_assert(sizeof(From) == sizeof(To), "BitCast types must be same size");
    return common::Read<To>(&value);
}

// counts the number of leading bits that are the same value
// as the sign bit
u32 CountLeadingSignBits(s32 value);

} // namespace common