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