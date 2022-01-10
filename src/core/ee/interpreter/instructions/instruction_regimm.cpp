#include <common/types.h>
#include <core/ee/interpreter/interpreter.h>
#include <core/system.h>

void EEInterpreter::bgez() {
    s32 offset = sign_extend<s32, 16>(inst.i.imm) << 2;

    if (GetReg<s64>(inst.i.rs) >= 0) {
        regs.next_pc = regs.pc + offset + 4;
        branch_delay = true;
    }
}

void EEInterpreter::bltz() {
    s32 offset = sign_extend<s32, 16>(inst.i.imm) << 2;

    if (GetReg<s64>(inst.i.rs) < 0) {
        regs.next_pc = regs.pc + offset + 4;
        branch_delay = true;
    }
}

void EEInterpreter::bltzl() {
    s32 offset = sign_extend<s32, 16>(inst.i.imm) << 2;

    if (GetReg<s64>(inst.i.rs) < 0) {
        regs.next_pc = regs.pc + offset + 4;
        branch_delay = true;
    } else {
        regs.pc += 4;
    }
}