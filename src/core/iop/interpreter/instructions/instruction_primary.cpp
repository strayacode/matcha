#include <common/types.h>
#include <core/iop/interpreter/interpreter.h>

void IOPInterpreter::slti() {
    SetReg(inst.i.rt, (s32)GetReg(inst.i.rs) < sign_extend<s32, 16>(inst.i.imm));
}

void IOPInterpreter::bne() {
    s32 offset = sign_extend<s32, 16>(inst.i.imm) << 2;

    if (GetReg(inst.i.rs) != GetReg(inst.i.rt)) {
        regs.next_pc = regs.pc + offset + 4;
        branch_delay = true;
    }
}

void IOPInterpreter::lui() {
    SetReg(inst.i.rt, inst.i.imm << 16);
}

void IOPInterpreter::ori() {
    SetReg(inst.i.rt, GetReg(inst.i.rs) | inst.i.imm);
}

void IOPInterpreter::beq() {
    s32 offset = sign_extend<s32, 16>(inst.i.imm) << 2;

    if (GetReg(inst.i.rs) == GetReg(inst.i.rt)) {
        regs.next_pc = regs.pc + offset + 4;
        branch_delay = true;
    }
}

void IOPInterpreter::lw() {
    if (!(cop0.gpr[12] & 0x10000)) {
        SetReg(inst.i.rt, ReadWord(GetReg(inst.i.rs) + sign_extend<s32, 16>(inst.i.imm)));
    }
}

void IOPInterpreter::andi() {
    SetReg(inst.i.rt, GetReg(inst.i.rs) & inst.i.imm);
}

void IOPInterpreter::addiu() {
    SetReg(inst.i.rt, GetReg(inst.i.rs) + sign_extend<s32, 16>(inst.i.imm));
}

void IOPInterpreter::addi() {
    SetReg(inst.i.rt, GetReg(inst.i.rs) + sign_extend<s32, 16>(inst.i.imm));
}

void IOPInterpreter::sw() {
    if (!(cop0.gpr[12] & 0x10000)) {
        WriteWord(GetReg(inst.i.rs) + sign_extend<s32, 16>(inst.i.imm), GetReg(inst.i.rt));
    }
}

void IOPInterpreter::sb() {
    if (!(cop0.gpr[12] & 0x10000)) {
        WriteByte(GetReg(inst.i.rs) + sign_extend<s32, 16>(inst.i.imm), GetReg(inst.i.rt));
    }
}

void IOPInterpreter::lb() {
    if (!(cop0.gpr[12] & 0x10000)) {
        SetReg(inst.i.rt, sign_extend<s32, 8>(ReadByte(GetReg(inst.i.rs) + sign_extend<s32, 16>(inst.i.imm))));
    }
}

void IOPInterpreter::jal() {
    SetReg(31, regs.pc + 8);
    regs.next_pc = (regs.pc & 0xF0000000) + (inst.j.offset << 2);
    branch_delay = true;
}

void IOPInterpreter::lh() {
    if (!(cop0.gpr[12] & 0x10000)) {
        u32 addr = regs.gpr[inst.i.rs] + sign_extend<s32, 16>(inst.i.imm);

        SetReg(inst.i.rt, sign_extend<s32, 16>(ReadHalf(addr)));
    }
}

void IOPInterpreter::j() {
    regs.next_pc = (regs.pc & 0xF0000000) + (inst.j.offset << 2);
    branch_delay = true;
}

void IOPInterpreter::lbu() {
    if (!(cop0.gpr[12] & 0x10000)) {
        SetReg(inst.i.rt, ReadByte(GetReg(inst.i.rs) + sign_extend<s32, 16>(inst.i.imm)));
    }
}

void IOPInterpreter::sltiu() {
    SetReg(inst.i.rt, GetReg(inst.i.rs) < (u32)sign_extend<s32, 16>(inst.i.imm));
}

void IOPInterpreter::lhu() {
    if (!(cop0.gpr[12] & 0x10000)) {
        u32 addr = GetReg(inst.i.rs) + sign_extend<s32, 16>(inst.i.imm);

        SetReg(inst.i.rt, ReadHalf(addr));
    }
}

void IOPInterpreter::blez() {
    if (static_cast<s32>(GetReg(inst.i.rs)) <= 0) {
        regs.next_pc = regs.pc + (sign_extend<s32, 16>(inst.i.imm) << 2) + 4;
        branch_delay = true;
    }
}

void IOPInterpreter::bgtz() {
    if (static_cast<s32>(GetReg(inst.i.rs)) > 0) {
        regs.next_pc = regs.pc + (sign_extend<s32, 16>(inst.i.imm) << 2) + 4;
        branch_delay = true;
    }
}

void IOPInterpreter::sh() {
    if (!(cop0.gpr[12] & 0x10000)) {
        u32 addr = GetReg(inst.i.rs) + sign_extend<s32, 16>(inst.i.imm);
        
        WriteHalf(addr, GetReg(inst.i.rt));
    }
}
