#include <common/types.h>
#include <core/ee/interpreter/interpreter.h>
#include <core/system.h>

void EEInterpreter::swc1() {
    WriteWord(GetReg<u32>(inst.i.rs) + (s16)inst.i.imm, system->ee_cop1.GetReg(inst.i.rt));
}