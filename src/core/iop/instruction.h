#pragma once

#include "common/types.h"

namespace iop {

union Instruction {
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

    Instruction(u32 data) : data(data) {};
    Instruction() : data(0) {};
};

} // namespace iop