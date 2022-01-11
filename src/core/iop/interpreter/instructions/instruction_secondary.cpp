#include <common/types.h>
#include <core/iop/interpreter/interpreter.h>

void IOPInterpreter::sll() {
    SetReg(inst.r.rd, GetReg(inst.r.rt) << inst.r.sa);
}

void IOPInterpreter::jr() {
    regs.next_pc = GetReg(inst.i.rs);
    branch_delay = true;
}

void IOPInterpreter::orr() {
    SetReg(inst.r.rd, GetReg(inst.r.rs) | GetReg(inst.r.rt));
}

void IOPInterpreter::addu() {
    SetReg(inst.r.rd, GetReg(inst.r.rs) + GetReg(inst.r.rt));
}

void IOPInterpreter::sltu() {
    SetReg(inst.r.rd, GetReg(inst.r.rs) < GetReg(inst.r.rt));
}

void IOPInterpreter::andd() {
    SetReg(inst.r.rd, GetReg(inst.r.rs) & GetReg(inst.r.rt));
}

void IOPInterpreter::slt() {
    SetReg(inst.r.rd, (s32)GetReg(inst.r.rs) < (s32)GetReg(inst.r.rt));
}

void IOPInterpreter::sra() {
    SetReg(inst.r.rd, (s32)GetReg(inst.r.rt) >> inst.r.sa);
}

void IOPInterpreter::srl() {
    SetReg(inst.r.rd, GetReg(inst.r.rt) >> inst.r.sa);
}

void IOPInterpreter::subu() {
    SetReg(inst.r.rd, GetReg(inst.r.rs) - GetReg(inst.r.rt));
}

void IOPInterpreter::divu() {
    if (GetReg(inst.i.rt) == 0) {
        regs.lo = 0xFFFFFFFF;
        regs.hi = GetReg(inst.i.rs);
    } else {
        regs.lo = GetReg(inst.i.rs) / GetReg(inst.i.rt);
        regs.hi = GetReg(inst.i.rs) % GetReg(inst.i.rt);
    }
}

void IOPInterpreter::mflo() {
    SetReg(inst.r.rd, regs.lo);
}

void IOPInterpreter::jalr() {
    u32 addr = GetReg(inst.r.rs);

    SetReg(inst.r.rd, regs.pc + 8);
    
    regs.next_pc = addr;
    branch_delay = true;
}

void IOPInterpreter::xorr() {
    SetReg(inst.r.rd, GetReg(inst.r.rs) ^ GetReg(inst.r.rt));
}

void IOPInterpreter::sllv() {
    SetReg(inst.r.rd, GetReg(inst.r.rt) << (GetReg(inst.r.rs) & 0x1F));
}

void IOPInterpreter::mfhi() {
    SetReg(inst.r.rd, regs.hi);
}

void IOPInterpreter::multu() {
    u64 result = (u64)GetReg(inst.i.rs) * (u64)GetReg(inst.i.rt);

    regs.lo = result & 0xFFFFFFFF;
    regs.hi = result >> 32;
}

void IOPInterpreter::mthi() {
    regs.hi = GetReg(inst.i.rs);
}

void IOPInterpreter::mtlo() {
    regs.lo = GetReg(inst.i.rs);
}

void IOPInterpreter::syscall_exception() {
    DoException(ExceptionType::Syscall);
}

void IOPInterpreter::mult() {
    s64 result = (s64)(s32)GetReg(inst.i.rs) * (s64)(s32)GetReg(inst.i.rt);

    regs.lo = result & 0xFFFFFFFF;
    regs.hi = result >> 32;
}

void IOPInterpreter::nor() {
    SetReg(inst.r.rd, 0xFFFFFFFF ^ (GetReg(inst.r.rs) | GetReg(inst.r.rt)));
}