#include "core/ee/cop0.h"

namespace ee {

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

void COP0::Reset() {
    for (int i = 0; i < 32; i++) {
        gpr[i] = 0;
    }

    cause.data = 0;
    index = 0;
    gpr[PRId] = 0x2e20;
}

u32 COP0::GetReg(int reg) {
    switch (reg) {
    case 13:
        return cause.data;
    case 9: case 12: case 14: case 15: case 30:
        return gpr[reg];
    default:
        common::Error("[ee::COP0] handle read r%d", reg);
    }

    return 0;
}

void COP0::SetReg(int reg, u32 value) {
    switch (reg) {
    case 0:
        index = value;
        break;
    case 2: 
        common::Log("[ee::COP0] entrylo0 write %08x", value);
        gpr[reg] = value;
        break;
    case 3:
        common::Log("[ee::COP0] entrylo1 write %08x", value);
        gpr[reg] = value;
        break;
    case 5: case 6: 
    case 9: case 10: case 12: case 16:
        gpr[reg] = value;
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
        gpr[reg] = value;
        break;
    case 14:
        // common::Log("[COP0] write EPC %08x", value);
        gpr[EPC] = value;
        break;
    default:
        common::Error("[ee::COP0] handle write r%d = %08x", reg, value);
    }
}

void COP0::CountUp() {
    gpr[Count]++;

    if (gpr[Count] == gpr[Compare]) {
        cause.timer_pending = true;
    }
}

} // namespace ee