#include "common/types.h"
#include "core/iop/interpreter/interpreter.h"
#include "core/system.h"

void IOPInterpreter::mfc0() {
    SetReg(inst.rt, cop0.GetReg(inst.rd));
}

void IOPInterpreter::mtc0() {
    cop0.SetReg(inst.rd, GetReg(inst.rt));
}

void IOPInterpreter::rfe() {
    // pop the 3 entry stack in sr
    u8 stack = cop0.gpr[12] & 0x3F;
    cop0.gpr[12] &= ~0xF;
    cop0.gpr[12] |= (stack >> 2);
}