#pragma once

#include <common/types.h>

struct IOPRegs {
    u32 gpr[32];
    u32 pc;
    u32 npc;
    u32 hi;
    u32 lo;
};