#include "common/types.h"
#include "core/iop/interpreter/interpreter.h"
#include "core/system.h"

void IOPInterpreter::mfc0() {
    SetReg(inst.r.rt, cop0.GetReg(inst.r.rd));
}

void IOPInterpreter::mtc0() {
    cop0.SetReg(inst.r.rd, GetReg(inst.r.rt));
}

void IOPInterpreter::rfe() {
    // pop the 3 entry stack in sr
    u8 stack = cop0.gpr[12] & 0x3F;
    cop0.gpr[12] &= ~0xF;
    cop0.gpr[12] |= (stack >> 2);
}