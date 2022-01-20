#pragma once

#include "common/types.h"

template <typename T, int size>
T sign_extend(T data) {
    struct {
        T data : size;
    } s;

    s.data = data;

    return s.data;
}

// counts the number of leading bits that are the same value
// as the sign bit
u32 CountLeadingSignBits(s32 value);