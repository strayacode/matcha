#pragma once

#include <array>
#include "common/types.h"
#include "core/iop/cop0.h"
#include "core/iop/cdvd.h"
#include "core/iop/intc.h"
#include "core/iop/interpreter.h"

struct System;

namespace iop {

struct Context {
    Context(System& system);
    
    void Reset();
    void Run(int cycles);

    u32 GetReg(int reg) {
        return gpr[reg];
    }

    void SetReg(int reg, u32 value) {
        if (reg) {
            gpr[reg] = value;
        }
    }

    u8 ReadByte(u32 addr);
    u16 ReadHalf(u32 addr);
    u32 ReadWord(u32 addr);

    void WriteByte(u32 addr, u8 value);
    void WriteHalf(u32 addr, u16 value);
    void WriteWord(u32 addr, u32 value);

    void RaiseInterrupt(bool value);

    std::array<u32, 32> gpr;
    u32 pc;
    u32 npc;
    u32 hi;
    u32 lo;

    COP0 cop0;
    CDVD cdvd;
    INTC intc;

private:
    Interpreter interpreter;
    System& system;
};

} // namespace iop