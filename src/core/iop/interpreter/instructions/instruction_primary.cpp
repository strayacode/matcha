#include <common/types.h>
#include <core/iop/interpreter/interpreter.h>

void IOPInterpreter::slti() {
    SetReg(inst.rt, (s32)GetReg(inst.rs) < sign_extend<s32, 16>(inst.imm));
}

void IOPInterpreter::bne() {
    s32 offset = sign_extend<s32, 16>(inst.imm) << 2;

    if (GetReg(inst.rs) != GetReg(inst.rt)) {
        regs.next_pc = regs.pc + offset + 4;
        branch_delay = true;
    }
}

void IOPInterpreter::lui() {
    SetReg(inst.rt, inst.imm << 16);
}

void IOPInterpreter::ori() {
    SetReg(inst.rt, GetReg(inst.rs) | inst.imm);
}

void IOPInterpreter::beq() {
    s32 offset = sign_extend<s32, 16>(inst.imm) << 2;

    if (GetReg(inst.rs) == GetReg(inst.rt)) {
        regs.next_pc = regs.pc + offset + 4;
        branch_delay = true;
    }
}

void IOPInterpreter::lw() {
    if (!(cop0.gpr[12] & 0x10000)) {
        SetReg(inst.rt, ReadWord(GetReg(inst.rs) + sign_extend<s32, 16>(inst.imm)));
    }
}

void IOPInterpreter::andi() {
    SetReg(inst.rt, GetReg(inst.rs) & inst.imm);
}

void IOPInterpreter::addiu() {
    SetReg(inst.rt, GetReg(inst.rs) + sign_extend<s32, 16>(inst.imm));
}

void IOPInterpreter::addi() {
    SetReg(inst.rt, GetReg(inst.rs) + sign_extend<s32, 16>(inst.imm));
}

void IOPInterpreter::sw() {
    if (!(cop0.gpr[12] & 0x10000)) {
        WriteWord(GetReg(inst.rs) + sign_extend<s32, 16>(inst.imm), GetReg(inst.rt));
    }
}

void IOPInterpreter::sb() {
    if (!(cop0.gpr[12] & 0x10000)) {
        WriteByte(GetReg(inst.rs) + sign_extend<s32, 16>(inst.imm), GetReg(inst.rt));
    }
}

void IOPInterpreter::lb() {
    if (!(cop0.gpr[12] & 0x10000)) {
        SetReg(inst.rt, sign_extend<s32, 8>(ReadByte(GetReg(inst.rs) + sign_extend<s32, 16>(inst.imm))));
    }
}

void IOPInterpreter::jal() {
    SetReg(31, regs.pc + 8);
    regs.next_pc = (regs.pc & 0xF0000000) + (inst.offset << 2);
    branch_delay = true;
}

void IOPInterpreter::lh() {
    if (!(cop0.gpr[12] & 0x10000)) {
        u32 addr = regs.gpr[inst.rs] + sign_extend<s32, 16>(inst.imm);

        SetReg(inst.rt, sign_extend<s32, 16>(ReadHalf(addr)));
    }
}

void IOPInterpreter::j() {
    regs.next_pc = (regs.pc & 0xF0000000) + (inst.offset << 2);
    branch_delay = true;
}

void IOPInterpreter::lbu() {
    if (!(cop0.gpr[12] & 0x10000)) {
        SetReg(inst.rt, ReadByte(GetReg(inst.rs) + sign_extend<s32, 16>(inst.imm)));
    }
}

void IOPInterpreter::sltiu() {
    SetReg(inst.rt, GetReg(inst.rs) < (u32)sign_extend<s32, 16>(inst.imm));
}

void IOPInterpreter::lhu() {
    if (!(cop0.gpr[12] & 0x10000)) {
        u32 addr = GetReg(inst.rs) + sign_extend<s32, 16>(inst.imm);

        SetReg(inst.rt, ReadHalf(addr));
    }
}

void IOPInterpreter::blez() {
    if (static_cast<s32>(GetReg(inst.rs)) <= 0) {
        regs.next_pc = regs.pc + (sign_extend<s32, 16>(inst.imm) << 2) + 4;
        branch_delay = true;
    }
}

void IOPInterpreter::bgtz() {
    if (static_cast<s32>(GetReg(inst.rs)) > 0) {
        regs.next_pc = regs.pc + (sign_extend<s32, 16>(inst.imm) << 2) + 4;
        branch_delay = true;
    }
}

void IOPInterpreter::sh() {
    if (!(cop0.gpr[12] & 0x10000)) {
        u32 addr = GetReg(inst.rs) + sign_extend<s32, 16>(inst.imm);
        
        WriteHalf(addr, GetReg(inst.rt));
    }
}

void IOPInterpreter::bcondz() {
    bool link = (inst.rt & 0x1E) == 0x10;
    bool ge = inst.rt & 0x1;
    bool branch = (static_cast<s32>(GetReg(inst.rs)) < 0) ^ ge;

    if (branch) {
        regs.next_pc = regs.pc + (sign_extend<s32, 16>(inst.imm) << 2) + 4;
        branch_delay = true;
    }

    if (link) {
        SetReg(31, regs.pc + 8);
    }
}

void IOPInterpreter::lwl() {
    u32 addr = GetReg(inst.rs) + inst.simm;
    u32 aligned_data = ReadWord(addr & ~0x3);
    u8 shift = (addr & 0x3) * 8;
    u32 mask = 0xFFFFFF >> shift;
    
    SetReg(inst.rt, (GetReg(inst.rt) & mask) | (aligned_data << (24 - shift)));
}

void IOPInterpreter::lwr() {
    u32 addr = GetReg(inst.rs) + inst.simm;
    u32 aligned_data = ReadWord(addr & ~0x3);
    u8 shift = (addr & 0x3) * 8;
    u32 mask = 0xFFFFFF00 << (24 - shift);
    
    SetReg(inst.rt, (GetReg(inst.rt) & mask) | (aligned_data >> shift));
}

void IOPInterpreter::swl() {
    u32 addr = GetReg(inst.rs) + inst.simm;
    u32 aligned_data = ReadWord(addr & ~0x3);
    u8 shift = (addr & 0x3) * 8;
    u32 mask = 0xFFFFFF00 << shift;
    u32 updated_data = (aligned_data & mask) | (GetReg(inst.rt) >> (24 - shift));

    WriteWord(addr & ~0x3, updated_data);
}

void IOPInterpreter::swr() {
    u32 addr = GetReg(inst.rs) + inst.simm;
    u32 aligned_data = ReadWord(addr & ~0x3);
    u8 shift = (addr & 0x3) * 8;
    u32 mask = 0xFFFFFF >> (24 - shift);
    u32 updated_data = (aligned_data & mask) | (GetReg(inst.rt) << shift);

    WriteWord(addr & ~0x3, updated_data);
}