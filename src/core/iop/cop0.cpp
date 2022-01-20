#include "core/iop/cop0.h"

enum COP0Regs {
    Index = 0,
    Random = 1,
    EntryLo0 = 2,
    EntryLo1 = 3,
    Context = 4,
    PageMask = 5,
    Wired = 6,
    BadVAddr = 8,
    Count = 9,
    EntryHi = 10,
    Compare = 11,
    Status = 12,
    Cause = 13,
    EPC = 14,
    PRId = 15,
    Config = 16,
    BadPAddr = 23,
    Debug = 24,
    Perf = 25,
    TagLo = 28,
    TagHi = 29,
    ErrorEPC = 30,
};

void IOPCOP0::Reset() {
    for (int i = 0; i < 32; i++) {
        gpr[i] = 0;
    }

    gpr[PRId] = 0x1F;
}

u32 IOPCOP0::GetReg(int reg) {
    switch (reg) {
    case 12: case 13: case 14: case 15:
        return gpr[reg];
    default:
        log_fatal("handle cop0 read %d", reg);
    }
}

void IOPCOP0::SetReg(int reg, u32 data) {
    switch (reg) {
    case 12: case 14:
        gpr[reg] = data;
        break;
    }
}