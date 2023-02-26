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
    u8 stack = cop0.status.data & 0x3f;
    cop0.status.data &= ~0xf;
    cop0.status.data |= stack >> 2;
}