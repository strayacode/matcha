#include <core/ee/cop0.h>

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

void EECOP0::Reset() {
    for (int i = 0; i < 32; i++) {
        gpr[i] = 0;
    }

    cause.data = 0;
    gpr[PRId] = 0x2E20;
}

u32 EECOP0::GetReg(int reg) {
    switch (reg) {
    case 13:
        return cause.data;
    case 9: case 12: case 14: case 15: case 30:
        return gpr[reg];
    default:
        common::Error("handle cop0 read %d", reg);
    }
}

void EECOP0::SetReg(int reg, u32 data) {
    switch (reg) {
    case 0: case 2: case 3: case 5: case 6: 
    case 9: case 10: case 12: case 16:
        gpr[reg] = data;
        break;
    case 13:
        // cause can't be written to.
        // this would allow some interrupt pending bits to possibly be set,
        // which can result in unintentional interrupts
        
        break;
    case 11:
        // writing to compare clears timer interrupt pending bit
        // in cause
        cause.timer_pending = false;
        gpr[reg] = data;
        break;
    case 14:
        // common::Warn("[COP0] write EPC %08x", data);
        gpr[EPC] = data;
        break;
    default:
        common::Error("handle cop0 write %d = %08x", reg, data);
    }
}

void EECOP0::CountUp() {
    gpr[Count]++;

    if (gpr[Count] == gpr[Compare]) {
        cause.timer_pending = true;
    }
}