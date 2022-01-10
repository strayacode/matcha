#pragma once

#include <common/types.h>

// inspired by DobieStation.
// although it might be slightly slower than a union
// it's cleaner
struct EERegs {
    u8 gpr[32 * sizeof(u64) * 2];
    u32 pc;
    u32 next_pc;
    u64 hi;
    u64 lo;
    u64 hi1;
    u64 lo1;
    u64 sa;
};