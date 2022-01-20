#include "common/arithmetic.h"

u32 CountLeadingSignBits(s32 value) {
    if (value < 0) {
        value = ~value;
    }

    // __builtin_clz is undefined for 0 but we can just handle this ourselves
    if (value == 0) {
        return 32;
    }

    return __builtin_clz(value);
}