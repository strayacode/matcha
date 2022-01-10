#include <common/types.h>
#include <core/ee/interpreter/interpreter.h>
#include <core/system.h>

void EEInterpreter::sll() {
    u32 result = GetReg<u32>(inst.r.rt) << inst.r.sa;
    SetReg<s64>(inst.r.rd, (s32)result);
}

void EEInterpreter::jr() {
    regs.next_pc = GetReg<u32>(inst.i.rs);
    branch_delay = true;
}

void EEInterpreter::sync() {
    
}

void EEInterpreter::jalr() {
    SetReg<u64>(inst.r.rd, regs.pc + 8);
    regs.next_pc = GetReg<u32>(inst.r.rs);
    branch_delay = true;
}

void EEInterpreter::daddu() {
    SetReg<u64>(inst.r.rd, GetReg<u64>(inst.r.rs) + GetReg<u64>(inst.r.rt));
}

void EEInterpreter::orr() {
    SetReg<u64>(inst.r.rd, GetReg<u64>(inst.r.rs) | GetReg<u64>(inst.r.rt));
}

void EEInterpreter::mult() {
    s64 result = GetReg<s32>(inst.r.rs) * GetReg<s32>(inst.r.rt);
    regs.lo = (s32)(result & 0xFFFFFFFF);
    regs.hi = (s32)(result >> 32);
    SetReg<u64>(inst.r.rd, regs.lo);
}

void EEInterpreter::divu() {
    if (GetReg<u32>(inst.r.rt) == 0) {
        regs.lo = 0xFFFFFFFFFFFFFFFF;
        regs.hi = sign_extend<s64, 32>(GetReg<u32>(inst.r.rs));
    } else {
        regs.lo = sign_extend<s64, 32>(GetReg<u32>(inst.r.rs) / GetReg<u32>(inst.r.rt));
        regs.hi = sign_extend<s64, 32>(GetReg<u32>(inst.r.rs) % GetReg<u32>(inst.r.rt));
    }
}

void EEInterpreter::break_exception() {
    // not sure what to do here
}

void EEInterpreter::mflo() {
    SetReg<u64>(inst.r.rd, regs.lo);
}

void EEInterpreter::srl() {
    u32 result = GetReg<u32>(inst.r.rt) >> inst.r.sa;
    SetReg<s64>(inst.r.rd, (s32)result);
}

void EEInterpreter::sra() {
    s32 result = GetReg<s32>(inst.r.rt) >> inst.r.sa;
    SetReg<s64>(inst.r.rd, result);
}

void EEInterpreter::slt() {
    SetReg<u64>(inst.r.rd, GetReg<s64>(inst.r.rs) < GetReg<s64>(inst.r.rt));
}

void EEInterpreter::addu() {
    s32 result = GetReg<s64>(inst.r.rs) + GetReg<s64>(inst.r.rt);
    SetReg<s64>(inst.r.rd, result);
}

void EEInterpreter::sltu() {
    SetReg<u64>(inst.r.rd, GetReg<u64>(inst.r.rs) < GetReg<u64>(inst.r.rt));
}

void EEInterpreter::andd() {
    SetReg<u64>(inst.r.rd, GetReg<u64>(inst.r.rs) & GetReg<u64>(inst.r.rt));
}

void EEInterpreter::movn() {
    if (GetReg<u64>(inst.r.rt)) {
        SetReg<u64>(inst.r.rd, GetReg<u64>(inst.r.rs));
    }
}

void EEInterpreter::subu() {
    s32 result = GetReg<s32>(inst.r.rs) - GetReg<s32>(inst.r.rt);
    SetReg<s64>(inst.r.rd, result);
}

void EEInterpreter::div() {
    if ((GetReg<s32>(inst.r.rs) == (s32)0x80000000) && (GetReg<s32>(inst.r.rt) == -1)) {
        regs.lo = 0x80000000;
        regs.hi = 0;
    } else if (GetReg<s32>(inst.r.rt) == 0) {
        log_fatal("can't divide by 0");
    } else {
        regs.lo = sign_extend<s64, 32>(GetReg<s32>(inst.r.rs) / GetReg<s32>(inst.r.rt));
        regs.hi = sign_extend<s64, 32>(GetReg<s32>(inst.r.rs) % GetReg<s32>(inst.r.rt));
    }
}

void EEInterpreter::mfhi() {
    SetReg<u64>(inst.r.rd, regs.hi);
}

void EEInterpreter::dsrav() {
    SetReg<s64>(inst.r.rd, GetReg<s64>(inst.r.rt) >> (GetReg<u8>(inst.r.rs) & 0x3F));
}

void EEInterpreter::dsll32() {
    SetReg<u64>(inst.r.rd, GetReg<u64>(inst.r.rt) << (32 + inst.r.sa));
}

void EEInterpreter::dsra32() {
    SetReg<s64>(inst.r.rd, GetReg<s64>(inst.r.rt) >> (32 + inst.r.sa));
}

void EEInterpreter::movz() {
    if (GetReg<u64>(inst.r.rt) == 0) {
        SetReg<u64>(inst.r.rd, GetReg<u64>(inst.r.rs));
    }
}

void EEInterpreter::dsllv() {
    SetReg<u64>(inst.r.rd, GetReg<u64>(inst.r.rt) << (GetReg<u8>(inst.r.rs) & 0x3F));
}

void EEInterpreter::sllv() {
    u32 result = GetReg<u32>(inst.r.rt) << (GetReg<u8>(inst.r.rs) & 0x1F);
    SetReg<s64>(inst.r.rd, sign_extend<s64, 32>(result));
}

void EEInterpreter::dsll() {
    SetReg<u64>(inst.r.rd, GetReg<u64>(inst.r.rt) << inst.r.sa);
}

void EEInterpreter::srav() {
    s32 result = GetReg<s32>(inst.r.rt) >> (GetReg<u8>(inst.r.rs) & 0x1F);
    SetReg<s64>(inst.r.rd, result);
}

void EEInterpreter::nor() {
    SetReg<u64>(inst.r.rd, ~(GetReg<u64>(inst.r.rs) | GetReg<u64>(inst.r.rt)));
}

void EEInterpreter::dsrl() {
    SetReg<u64>(inst.r.rd, GetReg<u64>(inst.r.rt) >> inst.r.sa);
}

void EEInterpreter::srlv() {
    s32 result = GetReg<u32>(inst.r.rt) >> (GetReg<u8>(inst.r.rs) & 0x1F);
    SetReg<s64>(inst.r.rd, result);
}

void EEInterpreter::dsrl32() {
    SetReg<u64>(inst.r.rd, GetReg<u64>(inst.r.rt) >> (32 + inst.r.sa));
}

void EEInterpreter::syscall() {
    DoException(0x80000180, ExceptionType::Syscall);
}