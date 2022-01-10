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