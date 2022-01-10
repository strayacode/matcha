#include <common/types.h>
#include <core/ee/interpreter/interpreter.h>
#include <core/system.h>

void EEInterpreter::divu1() {
    if (GetReg<u32>(inst.r.rt)) {
        regs.lo1 = sign_extend<s64, 32>(GetReg<u32>(inst.r.rs) / GetReg<u32>(inst.r.rt));
        regs.hi1 = sign_extend<s64, 32>(GetReg<u32>(inst.r.rs) % GetReg<u32>(inst.r.rt));
    } else {
        regs.lo1 = 0xFFFFFFFFFFFFFFFF;
        regs.hi1 = sign_extend<s64, 32>(GetReg<u32>(inst.r.rs));
    }
}

void EEInterpreter::mflo1() {
    SetReg<u64>(inst.r.rd, regs.lo1);
}

void EEInterpreter::mult1() {
    s64 result = GetReg<s32>(inst.r.rt) * GetReg<s32>(inst.r.rs);
    regs.lo1 = sign_extend<s64, 32>(result & 0xFFFFFFFF);
    regs.hi1 = sign_extend<s64, 32>(result >> 32);
    SetReg<u64>(inst.r.rd, regs.lo1);
}

void EEInterpreter::por() {
    SetReg<u128>(inst.r.rd, GetReg<u128>(inst.r.rs) | GetReg<u128>(inst.r.rt));
}

void EEInterpreter::padduw() {
    for (int i = 0; i < 4; i++) {
        u64 result = GetReg<u32>(inst.r.rs, i) + GetReg<u32>(inst.r.rt, i);

        if (result > 0xFFFFFFFF) {
            SetReg<u32>(inst.r.rd, 0xFFFFFFFF, i);
        } else {
            SetReg<u32>(inst.r.rd, result);
        }
    }
}