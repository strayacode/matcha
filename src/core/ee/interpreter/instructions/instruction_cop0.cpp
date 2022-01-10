#include <common/types.h>
#include <core/ee/interpreter/interpreter.h>
#include <core/system.h>

void EEInterpreter::mfc0() {
    SetReg<s64>(inst.r.rt, (s32)system->ee_cop0.GetReg(inst.r.rd));
}

void EEInterpreter::mtc0() {
    system->ee_cop0.SetReg(inst.r.rd, GetReg<u32>(inst.r.rt));
}