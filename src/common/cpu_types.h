#pragma once

#include "common/types.h"

// turns out both the r3000 and r5900
// use the same instruction encoding, so
// we make this common
union CPUInstruction {
    struct {
        u32 func : 6;
        u32 imm5 : 5;
        u32 rd : 5;
        u32 rt : 5;
        u32 rs : 5;
        u32 opcode : 6;
    };

    u32 data;
    s16 simm;
    u16 imm;
    u32 offset : 26;

    CPUInstruction(u32 data) : data(data) {};
    CPUInstruction() : data(0) {};
};

enum class InstructionTable {
    Primary,
    Secondary,
    RegImm,
    COP0,
    COP1,
    COP2,
    TLB,
    MMI,
    MMI1,
    MMI3,
};