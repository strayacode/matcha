#pragma once

#include <string>
#include "common/types.h"
#include "common/virtual_page_table.h"
#include "core/ee/cop0.h"
#include "core/ee/cop1.h"
#include "core/ee/interpreter.h"

struct System;

namespace ee {

struct Context {
    Context(System& system);

    void Reset();
    void Run(int cycles);

    // credit goes to DobieStation for the elegant way of accessing 128 bit registers
    template <typename T>
    T GetReg(int reg, int offset = 0) {
        return *(T*)&gpr[(reg * sizeof(u64) * 2) + (offset * sizeof(T))];
    }

    template <typename T>
    void SetReg(int reg, T value, int offset = 0) {
        if (reg) {
            *(T*)&gpr[(reg * sizeof(u64) * 2) + (offset * sizeof(T))] = value;
        }
    }

    template <typename T>
    T Read(VirtualAddress vaddr);

    void WriteByte(u32 addr, u8 value);
    void WriteHalf(u32 addr, u16 value);
    void WriteWord(u32 addr, u32 value);
    void WriteDouble(u32 addr, u64 value);
    void WriteQuad(u32 addr, u128 value);

    void RaiseInterrupt(int signal, bool value);
    std::string GetSyscallInfo(int index);

    std::array<u8, 512> gpr;
    u32 pc = 0;
    u32 npc = 0;
    u64 hi = 0;
    u64 lo = 0;
    u64 hi1 = 0;
    u64 lo1 = 0;
    u64 sa = 0;

    COP0 cop0;
    COP1 cop1;

private:
    common::VirtualPageTable vtlb;
    Interpreter interpreter;
    System& system;
};

} // namespace ee