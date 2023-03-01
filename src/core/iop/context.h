#pragma once

#include <array>
#include "common/types.h"
#include "common/virtual_page_table.h"
#include "core/iop/cop0.h"
#include "core/iop/cdvd.h"
#include "core/iop/sio2.h"
#include "core/iop/dmac.h"
#include "core/iop/timers.h"
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

    template <typename T>
    T Read(VirtualAddress vaddr);

    template <typename T>
    void Write(VirtualAddress vaddr, T value);

    void RaiseInterrupt(bool value);

    std::array<u32, 32> gpr;
    u32 pc;
    u32 npc;
    u32 hi;
    u32 lo;

    COP0 cop0;
    CDVD cdvd;
    DMAC dmac;
    Timers timers;
    INTC intc;
    SIO2 sio2;

private:
    u32 ReadIO(u32 paddr);
    void WriteIO(u32 paddr, u32 value);

    common::VirtualPageTable vtlb;
    Interpreter interpreter;
    System& system;
};

} // namespace iop