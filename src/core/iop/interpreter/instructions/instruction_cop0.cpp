#include <common/types.h>
#include <core/iop/interpreter/interpreter.h>
#include <core/system.h>

void IOPInterpreter::mfc0() {
    SetReg(inst.r.rt, cop0.GetReg(inst.r.rd));
}

void IOPInterpreter::mtc0() {
    cop0.SetReg(inst.r.rd, GetReg(inst.r.rt));
}