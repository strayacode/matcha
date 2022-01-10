#include <common/types.h>
#include <core/ee/interpreter/interpreter.h>
#include <core/system.h>

void EEInterpreter::slti() {
    SetReg<u64>(inst.i.rt, GetReg<s64>(inst.i.rs) < sign_extend<s64, 16>(inst.i.imm));
}

void EEInterpreter::bne() {
    s32 offset = sign_extend<s32, 16>(inst.i.imm) << 2;

    if (GetReg<u64>(inst.i.rs) != GetReg<u64>(inst.i.rt)) {
        regs.next_pc = regs.pc + offset + 4;
        branch_delay = true;
    }
}

void EEInterpreter::lui() {
    SetReg<u64>(inst.i.rt, sign_extend<s64, 32>(inst.i.imm << 16));
}

void EEInterpreter::ori() {
    SetReg<u64>(inst.i.rt, GetReg<u64>(inst.i.rs) | inst.i.imm);
}

void EEInterpreter::addiu() {
    s32 result = GetReg<s64>(inst.i.rs) + (s16)inst.i.imm;
    SetReg<s64>(inst.i.rt, result);
}

void EEInterpreter::sw() {
    WriteWord(GetReg<u32>(inst.i.rs) + (s16)inst.i.imm, GetReg<u32>(inst.i.rt));
}

void EEInterpreter::sd() {
    WriteDouble(GetReg<u32>(inst.i.rs) + (s16)inst.i.imm, GetReg<u64>(inst.i.rt));
}

void EEInterpreter::jal() {
    SetReg<u64>(31, regs.pc + 8);
    regs.next_pc = ((regs.pc + 4) & 0xF0000000) + (inst.j.offset << 2);
    branch_delay = true;
}

void EEInterpreter::andi() {
    SetReg<u64>(inst.i.rt, GetReg<u64>(inst.i.rs) & inst.i.imm);
}

void EEInterpreter::beq() {
    s32 offset = sign_extend<s32, 16>(inst.i.imm) << 2;

    if (GetReg<u64>(inst.i.rs) == GetReg<u64>(inst.i.rt)) {
        regs.next_pc = regs.pc + offset + 4;
        branch_delay = true;
    }
}

void EEInterpreter::beql() {
    s32 offset = sign_extend<s32, 16>(inst.i.imm) << 2;

    if (GetReg<u64>(inst.i.rs) == GetReg<u64>(inst.i.rt)) {
        regs.next_pc = regs.pc + offset + 4;
        branch_delay = true;
    } else {
        regs.pc += 4;
    }
}

void EEInterpreter::sltiu() {
    SetReg<u64>(inst.i.rt, GetReg<u64>(inst.i.rs) < (u64)sign_extend<s64, 16>(inst.i.imm));
}

void EEInterpreter::bnel() {
    s32 offset = sign_extend<s32, 16>(inst.i.imm) << 2;

    if (GetReg<u64>(inst.i.rs) != GetReg<u64>(inst.i.rt)) {
        regs.next_pc = regs.pc + offset + 4;
        branch_delay = true;
    } else {
        regs.pc += 4;
    }
}

void EEInterpreter::lb() {
    SetReg<s64>(inst.i.rt, (s8)ReadByte(GetReg<u32>(inst.i.rs) + (s16)inst.i.imm));
}

void EEInterpreter::lbu() {
    SetReg<u64>(inst.i.rt, ReadByte(GetReg<u32>(inst.i.rs) + (s16)inst.i.imm));
}

void EEInterpreter::ld() {
    SetReg<u64>(inst.i.rt, ReadDouble(GetReg<u32>(inst.i.rs) + (s16)inst.i.imm));
}

void EEInterpreter::j() {
    regs.next_pc = ((regs.pc + 4) & 0xF0000000) + (inst.j.offset << 2);
    branch_delay = true;
}

void EEInterpreter::lw() {
    SetReg<s64>(inst.i.rt, (s32)ReadWord(GetReg<u32>(inst.i.rs) + (s16)inst.i.imm));
}

void EEInterpreter::sb() {
    WriteByte(GetReg<u32>(inst.i.rs) + (s16)inst.i.imm, GetReg<u8>(inst.i.rt));
}

void EEInterpreter::blez() {
    s32 offset = sign_extend<s32, 16>(inst.i.imm) << 2;

    if (GetReg<s64>(inst.i.rs) <= 0) {
        regs.next_pc = regs.pc + offset + 4;
        branch_delay = true;
    }
}

void EEInterpreter::lhu() {
    SetReg<u64>(inst.i.rt, ReadHalf(GetReg<u32>(inst.i.rs) + (s16)inst.i.imm));
}

void EEInterpreter::bgtz() {
    s32 offset = sign_extend<s32, 16>(inst.i.imm) << 2;

    if (GetReg<s64>(inst.i.rs) > 0) {
        regs.next_pc = regs.pc + offset + 4;
        branch_delay = true;
    }
}

void EEInterpreter::sh() {
    WriteHalf(GetReg<u32>(inst.i.rs) + (s16)inst.i.imm, GetReg<u16>(inst.i.rt));
}

void EEInterpreter::xori() {
    SetReg<u64>(inst.i.rt, GetReg<u64>(inst.i.rs) ^ inst.i.imm);
}

void EEInterpreter::daddiu() {
    SetReg<s64>(inst.i.rt, GetReg<s64>(inst.i.rs) + sign_extend<s64, 16>(inst.i.imm));
}

void EEInterpreter::sq() {
    u128 reg = GetReg<u128>(inst.i.rt);
    u32 addr = GetReg<u32>(inst.i.rs) + (s16)inst.i.imm;

    WriteQuad(addr, reg);
}

void EEInterpreter::lq() {
    u128 data;
    u32 addr = GetReg<u32>(inst.i.rs) + sign_extend<s32, 16>(inst.i.imm);

    data.i.lo = ReadDouble(addr);
    data.i.hi = ReadDouble(addr + 8);

    SetReg<u128>(inst.r.rt, data);
}

void EEInterpreter::lh() {
    SetReg<s64>(inst.i.rt, (s16)ReadHalf(GetReg<u32>(inst.i.rs) + (s16)inst.i.imm));
}

void EEInterpreter::cache() {
    // handle later
}

void EEInterpreter::lwu() {
    SetReg<u64>(inst.i.rt, ReadWord(GetReg<u32>(inst.i.rs) + (s16)inst.i.imm));
}

void EEInterpreter::ldl() {
    const u64 mask[8] = {
        0x00FFFFFFFFFFFFFF, 0x0000FFFFFFFFFFFF, 0x000000FFFFFFFFFF, 0x00000000FFFFFFFF,
        0x0000000000FFFFFF, 0x000000000000FFFF, 0x00000000000000FF, 0x0000000000000000,
    };

    const u8 shift[8] = {56, 48, 40, 32, 24, 16, 8, 0};

    u32 addr = GetReg<u32>(inst.i.rs) + (s16)inst.i.imm;
    int index = addr & 0x7;
    u64 data = ReadDouble(addr & ~0x7);
    u64 reg = GetReg<u64>(inst.i.rt);
    SetReg<u64>(inst.i.rt, (reg & mask[index]) | (data << shift[index]));
}

void EEInterpreter::ldr() {
    const u64 mask[8] = {
        0x0000000000000000, 0xFF00000000000000, 0xFFFF000000000000, 0xFFFFFF0000000000,
        0xFFFFFFFF00000000, 0xFFFFFFFFFF000000, 0xFFFFFFFFFFFF0000, 0xFFFFFFFFFFFFFF00,
    };

    const u8 shift[8] = {0, 8, 16, 24, 32, 40, 48, 56};

    u32 addr = GetReg<u32>(inst.i.rs) + (s16)inst.i.imm;
    int index = addr & 0x7;
    u64 data = ReadDouble(addr & ~0x7);
    u64 reg = GetReg<u64>(inst.i.rt);
    SetReg<u64>(inst.i.rt, (reg & mask[index]) | (data >> shift[index]));
}

void EEInterpreter::sdl() {
    const u64 mask[8] = {
        0xFFFFFFFFFFFFFF00, 0xFFFFFFFFFFFF0000, 0xFFFFFFFFFF000000, 0xFFFFFFFF00000000,
        0xFFFFFF0000000000, 0xFFFF000000000000, 0xFF00000000000000, 0x0000000000000000,
    };

    const u8 shift[8] = {56, 48, 40, 32, 24, 16, 8, 0};

    u32 addr = GetReg<u32>(inst.i.rs) + (s16)inst.i.imm;
    int index = addr & 0x7;

    u64 data = ReadDouble(addr & ~0x7);
    u64 reg = GetReg<u64>(inst.i.rt);
    WriteDouble(addr & ~0x7, (reg >> shift[index]) | (data & mask[index]));
}

void EEInterpreter::sdr() {
    const u64 mask[8] = {
        0x0000000000000000, 0x00000000000000ff, 0x000000000000ffff, 0x0000000000ffffff,
        0x00000000ffffffff, 0x000000ffffffffff, 0x0000ffffffffffff, 0x00ffffffffffffff,
    };

    const u8 shift[8] = {0, 8, 16, 24, 32, 40, 48, 56};

    u32 addr = GetReg<u32>(inst.i.rs) + (s16)inst.i.imm;
    int index = addr & 0x7;

    u64 data = ReadDouble(addr & ~0x7);
    u64 reg = GetReg<u64>(inst.i.rt);
    WriteDouble(addr & ~0x7, (reg << shift[index]) | (data & mask[index]));
}