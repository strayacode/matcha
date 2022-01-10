#include "common/types.h"
#include "common/arithmetic.h"
#include "core/ee/ee_interpreter.h"
#include "core/ee/disassembler.h"

// COP0 instructions
void EEInterpreter::mfc0(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<s64>(inst.r.rt, static_cast<s32>(cpu.cop0.GetReg(inst.r.rd)));
}

void EEInterpreter::mtc0(EECore& cpu, CPUInstruction inst) {
    cpu.cop0.SetReg(inst.r.rd, cpu.GetReg<u32>(inst.r.rt));
}

// COP1 instructions
void EEInterpreter::swc1(EECore& cpu, CPUInstruction inst) {
    cpu.WriteWord(cpu.GetReg<u32>(inst.i.rs) + static_cast<s16>(inst.i.imm), cpu.cop1.GetReg(inst.i.rt));
}

// COP2 instructions
void EEInterpreter::cfc2(EECore& cpu, CPUInstruction inst) {
    // handle vu stuff
}

void EEInterpreter::ctc2(EECore& cpu, CPUInstruction inst) {
    // handle vu stuff
}

// MMI instructions
void EEInterpreter::divu1(EECore& cpu, CPUInstruction inst) {
    if (cpu.GetReg<u32>(inst.r.rt)) {
        cpu.regs.lo1 = sign_extend<s64, 32>(cpu.GetReg<u32>(inst.r.rs) / cpu.GetReg<u32>(inst.r.rt));
        cpu.regs.hi1 = sign_extend<s64, 32>(cpu.GetReg<u32>(inst.r.rs) % cpu.GetReg<u32>(inst.r.rt));
    } else {
        cpu.regs.lo1 = 0xFFFFFFFFFFFFFFFF;
        cpu.regs.hi1 = sign_extend<s64, 32>(cpu.GetReg<u32>(inst.r.rs));
    }
}

void EEInterpreter::mflo1(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<u64>(inst.r.rd, cpu.regs.lo1);
}

void EEInterpreter::mult1(EECore& cpu, CPUInstruction inst) {
    s64 result = cpu.GetReg<s32>(inst.r.rt) * cpu.GetReg<s32>(inst.r.rs);
    cpu.regs.lo1 = sign_extend<s64, 32>(result & 0xFFFFFFFF);
    cpu.regs.hi1 = sign_extend<s64, 32>(result >> 32);
    cpu.SetReg<u64>(inst.r.rd, cpu.regs.lo1);
}

void EEInterpreter::por(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<u128>(inst.r.rd, cpu.GetReg<u128>(inst.r.rs) | cpu.GetReg<u128>(inst.r.rt));
}

void EEInterpreter::padduw(EECore& cpu, CPUInstruction inst) {
    for (int i = 0; i < 4; i++) {
        u64 result = cpu.GetReg<u32>(inst.r.rs, i) + cpu.GetReg<u32>(inst.r.rt, i);

        if (result > 0xFFFFFFFF) {
            cpu.SetReg<u32>(inst.r.rd, 0xFFFFFFFF, i);
        } else {
            cpu.SetReg<u32>(inst.r.rd, result);
        }
    }
}

// primary instructions
void EEInterpreter::slti(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<u64>(inst.i.rt, cpu.GetReg<s64>(inst.i.rs) < sign_extend<s64, 16>(inst.i.imm));
}

void EEInterpreter::bne(EECore& cpu, CPUInstruction inst) {
    s32 offset = sign_extend<s32, 16>(inst.i.imm) << 2;

    if (cpu.GetReg<u64>(inst.i.rs) != cpu.GetReg<u64>(inst.i.rt)) {
        cpu.regs.next_pc = cpu.regs.pc + offset + 4;
        cpu.branch_delay = true;
    }
}

void EEInterpreter::lui(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<u64>(inst.i.rt, sign_extend<s64, 32>(inst.i.imm << 16));
}

void EEInterpreter::ori(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<u64>(inst.i.rt, cpu.GetReg<u64>(inst.i.rs) | inst.i.imm);
}

void EEInterpreter::addiu(EECore& cpu, CPUInstruction inst) {
    s32 result = cpu.GetReg<s64>(inst.i.rs) + (s16)inst.i.imm;
    cpu.SetReg<s64>(inst.i.rt, result);
}

void EEInterpreter::sw(EECore& cpu, CPUInstruction inst) {
    cpu.WriteWord(cpu.GetReg<u32>(inst.i.rs) + (s16)inst.i.imm, cpu.GetReg<u32>(inst.i.rt));
}

void EEInterpreter::sd(EECore& cpu, CPUInstruction inst) {
    cpu.WriteDouble(cpu.GetReg<u32>(inst.i.rs) + (s16)inst.i.imm, cpu.GetReg<u64>(inst.i.rt));
}

void EEInterpreter::jal(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<u64>(31, cpu.regs.pc + 8);
    cpu.regs.next_pc = ((cpu.regs.pc + 4) & 0xF0000000) + (inst.j.offset << 2);
    cpu.branch_delay = true;
}

void EEInterpreter::andi(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<u64>(inst.i.rt, cpu.GetReg<u64>(inst.i.rs) & inst.i.imm);
}

void EEInterpreter::beq(EECore& cpu, CPUInstruction inst) {
    s32 offset = sign_extend<s32, 16>(inst.i.imm) << 2;

    if (cpu.GetReg<u64>(inst.i.rs) == cpu.GetReg<u64>(inst.i.rt)) {
        cpu.regs.next_pc = cpu.regs.pc + offset + 4;
        cpu.branch_delay = true;
    }
}

void EEInterpreter::beql(EECore& cpu, CPUInstruction inst) {
    s32 offset = sign_extend<s32, 16>(inst.i.imm) << 2;

    if (cpu.GetReg<u64>(inst.i.rs) == cpu.GetReg<u64>(inst.i.rt)) {
        cpu.regs.next_pc = cpu.regs.pc + offset + 4;
        cpu.branch_delay = true;
    } else {
        cpu.regs.pc += 4;
    }
}

void EEInterpreter::sltiu(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<u64>(inst.i.rt, cpu.GetReg<u64>(inst.i.rs) < (u64)sign_extend<s64, 16>(inst.i.imm));
}

void EEInterpreter::bnel(EECore& cpu, CPUInstruction inst) {
    s32 offset = sign_extend<s32, 16>(inst.i.imm) << 2;

    if (cpu.GetReg<u64>(inst.i.rs) != cpu.GetReg<u64>(inst.i.rt)) {
        cpu.regs.next_pc = cpu.regs.pc + offset + 4;
        cpu.branch_delay = true;
    } else {
        cpu.regs.pc += 4;
    }
}

void EEInterpreter::lb(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<s64>(inst.i.rt, (s8)cpu.ReadByte(cpu.GetReg<u32>(inst.i.rs) + (s16)inst.i.imm));
}

void EEInterpreter::lbu(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<u64>(inst.i.rt, cpu.ReadByte(cpu.GetReg<u32>(inst.i.rs) + (s16)inst.i.imm));
}

void EEInterpreter::ld(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<u64>(inst.i.rt, cpu.ReadDouble(cpu.GetReg<u32>(inst.i.rs) + (s16)inst.i.imm));
}

void EEInterpreter::j(EECore& cpu, CPUInstruction inst) {
    cpu.regs.next_pc = ((cpu.regs.pc + 4) & 0xF0000000) + (inst.j.offset << 2);
    cpu.branch_delay = true;
}

void EEInterpreter::lw(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<s64>(inst.i.rt, (s32)cpu.ReadWord(cpu.GetReg<u32>(inst.i.rs) + (s16)inst.i.imm));
}

void EEInterpreter::sb(EECore& cpu, CPUInstruction inst) {
    cpu.WriteByte(cpu.GetReg<u32>(inst.i.rs) + (s16)inst.i.imm, cpu.GetReg<u8>(inst.i.rt));
}

void EEInterpreter::blez(EECore& cpu, CPUInstruction inst) {
    s32 offset = sign_extend<s32, 16>(inst.i.imm) << 2;

    if (cpu.GetReg<s64>(inst.i.rs) <= 0) {
        cpu.regs.next_pc = cpu.regs.pc + offset + 4;
        cpu.branch_delay = true;
    }
}

void EEInterpreter::lhu(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<u64>(inst.i.rt, cpu.ReadHalf(cpu.GetReg<u32>(inst.i.rs) + (s16)inst.i.imm));
}

void EEInterpreter::bgtz(EECore& cpu, CPUInstruction inst) {
    s32 offset = sign_extend<s32, 16>(inst.i.imm) << 2;

    if (cpu.GetReg<s64>(inst.i.rs) > 0) {
        cpu.regs.next_pc = cpu.regs.pc + offset + 4;
        cpu.branch_delay = true;
    }
}

void EEInterpreter::sh(EECore& cpu, CPUInstruction inst) {
    cpu.WriteHalf(cpu.GetReg<u32>(inst.i.rs) + (s16)inst.i.imm, cpu.GetReg<u16>(inst.i.rt));
}

void EEInterpreter::xori(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<u64>(inst.i.rt, cpu.GetReg<u64>(inst.i.rs) ^ inst.i.imm);
}

void EEInterpreter::daddiu(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<s64>(inst.i.rt, cpu.GetReg<s64>(inst.i.rs) + sign_extend<s64, 16>(inst.i.imm));
}

void EEInterpreter::sq(EECore& cpu, CPUInstruction inst) {
    u128 reg = cpu.GetReg<u128>(inst.i.rt);
    u32 addr = cpu.GetReg<u32>(inst.i.rs) + (s16)inst.i.imm;

    cpu.WriteQuad(addr, reg);
}

void EEInterpreter::lq(EECore& cpu, CPUInstruction inst) {
    u128 data;
    u32 addr = cpu.GetReg<u32>(inst.i.rs) + sign_extend<s32, 16>(inst.i.imm);

    data.i.lo = cpu.ReadDouble(addr);
    data.i.hi = cpu.ReadDouble(addr + 8);

    cpu.SetReg<u128>(inst.r.rt, data);
}

void EEInterpreter::lh(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<s64>(inst.i.rt, (s16)cpu.ReadHalf(cpu.GetReg<u32>(inst.i.rs) + (s16)inst.i.imm));
}

void EEInterpreter::cache(EECore& cpu, CPUInstruction inst) {
    // handle later
}

void EEInterpreter::lwu(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<u64>(inst.i.rt, cpu.ReadWord(cpu.GetReg<u32>(inst.i.rs) + (s16)inst.i.imm));
}

void EEInterpreter::ldl(EECore& cpu, CPUInstruction inst) {
    const u64 mask[8] = {
        0x00FFFFFFFFFFFFFF, 0x0000FFFFFFFFFFFF, 0x000000FFFFFFFFFF, 0x00000000FFFFFFFF,
        0x0000000000FFFFFF, 0x000000000000FFFF, 0x00000000000000FF, 0x0000000000000000,
    };

    const u8 shift[8] = {56, 48, 40, 32, 24, 16, 8, 0};

    u32 addr = cpu.GetReg<u32>(inst.i.rs) + (s16)inst.i.imm;
    int index = addr & 0x7;
    u64 data = cpu.ReadDouble(addr & ~0x7);
    u64 reg = cpu.GetReg<u64>(inst.i.rt);
    cpu.SetReg<u64>(inst.i.rt, (reg & mask[index]) | (data << shift[index]));
}

void EEInterpreter::ldr(EECore& cpu, CPUInstruction inst) {
    const u64 mask[8] = {
        0x0000000000000000, 0xFF00000000000000, 0xFFFF000000000000, 0xFFFFFF0000000000,
        0xFFFFFFFF00000000, 0xFFFFFFFFFF000000, 0xFFFFFFFFFFFF0000, 0xFFFFFFFFFFFFFF00,
    };

    const u8 shift[8] = {0, 8, 16, 24, 32, 40, 48, 56};

    u32 addr = cpu.GetReg<u32>(inst.i.rs) + (s16)inst.i.imm;
    int index = addr & 0x7;
    u64 data = cpu.ReadDouble(addr & ~0x7);
    u64 reg = cpu.GetReg<u64>(inst.i.rt);
    cpu.SetReg<u64>(inst.i.rt, (reg & mask[index]) | (data >> shift[index]));
}

void EEInterpreter::sdl(EECore& cpu, CPUInstruction inst) {
    const u64 mask[8] = {
        0xFFFFFFFFFFFFFF00, 0xFFFFFFFFFFFF0000, 0xFFFFFFFFFF000000, 0xFFFFFFFF00000000,
        0xFFFFFF0000000000, 0xFFFF000000000000, 0xFF00000000000000, 0x0000000000000000,
    };

    const u8 shift[8] = {56, 48, 40, 32, 24, 16, 8, 0};

    u32 addr = cpu.GetReg<u32>(inst.i.rs) + (s16)inst.i.imm;
    int index = addr & 0x7;

    u64 data = cpu.ReadDouble(addr & ~0x7);
    u64 reg = cpu.GetReg<u64>(inst.i.rt);
    cpu.WriteDouble(addr & ~0x7, (reg >> shift[index]) | (data & mask[index]));
}

void EEInterpreter::sdr(EECore& cpu, CPUInstruction inst) {
    const u64 mask[8] = {
        0x0000000000000000, 0x00000000000000ff, 0x000000000000ffff, 0x0000000000ffffff,
        0x00000000ffffffff, 0x000000ffffffffff, 0x0000ffffffffffff, 0x00ffffffffffffff,
    };

    const u8 shift[8] = {0, 8, 16, 24, 32, 40, 48, 56};

    u32 addr = cpu.GetReg<u32>(inst.i.rs) + (s16)inst.i.imm;
    int index = addr & 0x7;

    u64 data = cpu.ReadDouble(addr & ~0x7);
    u64 reg = cpu.GetReg<u64>(inst.i.rt);
    cpu.WriteDouble(addr & ~0x7, (reg << shift[index]) | (data & mask[index]));
}

// regimm instructions
void EEInterpreter::bgez(EECore& cpu, CPUInstruction inst) {
    s32 offset = sign_extend<s32, 16>(inst.i.imm) << 2;

    if (cpu.GetReg<s64>(inst.i.rs) >= 0) {
        cpu.regs.next_pc = cpu.regs.pc + offset + 4;
        cpu.branch_delay = true;
    }
}

void EEInterpreter::bltz(EECore& cpu, CPUInstruction inst) {
    s32 offset = sign_extend<s32, 16>(inst.i.imm) << 2;

    if (cpu.GetReg<s64>(inst.i.rs) < 0) {
        cpu.regs.next_pc = cpu.regs.pc + offset + 4;
        cpu.branch_delay = true;
    }
}

void EEInterpreter::bltzl(EECore& cpu, CPUInstruction inst) {
    s32 offset = sign_extend<s32, 16>(inst.i.imm) << 2;

    if (cpu.GetReg<s64>(inst.i.rs) < 0) {
        cpu.regs.next_pc = cpu.regs.pc + offset + 4;
        cpu.branch_delay = true;
    } else {
        cpu.regs.pc += 4;
    }
}

// secondary instructions
void EEInterpreter::sll(EECore& cpu, CPUInstruction inst) {
    u32 result = cpu.GetReg<u32>(inst.r.rt) << inst.r.sa;
    cpu.SetReg<s64>(inst.r.rd, (s32)result);
}

void EEInterpreter::jr(EECore& cpu, CPUInstruction inst) {
    cpu.regs.next_pc = cpu.GetReg<u32>(inst.i.rs);
    cpu.branch_delay = true;
}

void EEInterpreter::sync(EECore& cpu, CPUInstruction inst) {
    
}

void EEInterpreter::jalr(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<u64>(inst.r.rd, cpu.regs.pc + 8);
    cpu.regs.next_pc = cpu.GetReg<u32>(inst.r.rs);
    cpu.branch_delay = true;
}

void EEInterpreter::daddu(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<u64>(inst.r.rd, cpu.GetReg<u64>(inst.r.rs) + cpu.GetReg<u64>(inst.r.rt));
}

void EEInterpreter::orr(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<u64>(inst.r.rd, cpu.GetReg<u64>(inst.r.rs) | cpu.GetReg<u64>(inst.r.rt));
}

void EEInterpreter::mult(EECore& cpu, CPUInstruction inst) {
    s64 result = cpu.GetReg<s32>(inst.r.rs) * cpu.GetReg<s32>(inst.r.rt);
    cpu.regs.lo = (s32)(result & 0xFFFFFFFF);
    cpu.regs.hi = (s32)(result >> 32);
    cpu.SetReg<u64>(inst.r.rd, cpu.regs.lo);
}

void EEInterpreter::divu(EECore& cpu, CPUInstruction inst) {
    if (cpu.GetReg<u32>(inst.r.rt) == 0) {
        cpu.regs.lo = 0xFFFFFFFFFFFFFFFF;
        cpu.regs.hi = sign_extend<s64, 32>(cpu.GetReg<u32>(inst.r.rs));
    } else {
        cpu.regs.lo = sign_extend<s64, 32>(cpu.GetReg<u32>(inst.r.rs) / cpu.GetReg<u32>(inst.r.rt));
        cpu.regs.hi = sign_extend<s64, 32>(cpu.GetReg<u32>(inst.r.rs) % cpu.GetReg<u32>(inst.r.rt));
    }
}

void EEInterpreter::break_exception(EECore& cpu, CPUInstruction inst) {
    // not sure what to do here
}

void EEInterpreter::mflo(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<u64>(inst.r.rd, cpu.regs.lo);
}

void EEInterpreter::srl(EECore& cpu, CPUInstruction inst) {
    u32 result = cpu.GetReg<u32>(inst.r.rt) >> inst.r.sa;
    cpu.SetReg<s64>(inst.r.rd, (s32)result);
}

void EEInterpreter::sra(EECore& cpu, CPUInstruction inst) {
    s32 result = cpu.GetReg<s32>(inst.r.rt) >> inst.r.sa;
    cpu.SetReg<s64>(inst.r.rd, result);
}

void EEInterpreter::slt(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<u64>(inst.r.rd, cpu.GetReg<s64>(inst.r.rs) < cpu.GetReg<s64>(inst.r.rt));
}

void EEInterpreter::addu(EECore& cpu, CPUInstruction inst) {
    s32 result = cpu.GetReg<s64>(inst.r.rs) + cpu.GetReg<s64>(inst.r.rt);
    cpu.SetReg<s64>(inst.r.rd, result);
}

void EEInterpreter::sltu(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<u64>(inst.r.rd, cpu.GetReg<u64>(inst.r.rs) < cpu.GetReg<u64>(inst.r.rt));
}

void EEInterpreter::andd(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<u64>(inst.r.rd, cpu.GetReg<u64>(inst.r.rs) & cpu.GetReg<u64>(inst.r.rt));
}

void EEInterpreter::movn(EECore& cpu, CPUInstruction inst) {
    if (cpu.GetReg<u64>(inst.r.rt)) {
        cpu.SetReg<u64>(inst.r.rd, cpu.GetReg<u64>(inst.r.rs));
    }
}

void EEInterpreter::subu(EECore& cpu, CPUInstruction inst) {
    s32 result = cpu.GetReg<s32>(inst.r.rs) - cpu.GetReg<s32>(inst.r.rt);
    cpu.SetReg<s64>(inst.r.rd, result);
}

void EEInterpreter::div(EECore& cpu, CPUInstruction inst) {
    if ((cpu.GetReg<s32>(inst.r.rs) == (s32)0x80000000) && (cpu.GetReg<s32>(inst.r.rt) == -1)) {
        cpu.regs.lo = 0x80000000;
        cpu.regs.hi = 0;
    } else if (cpu.GetReg<s32>(inst.r.rt) == 0) {
        log_fatal("can't divide by 0");
    } else {
        cpu.regs.lo = sign_extend<s64, 32>(cpu.GetReg<s32>(inst.r.rs) / cpu.GetReg<s32>(inst.r.rt));
        cpu.regs.hi = sign_extend<s64, 32>(cpu.GetReg<s32>(inst.r.rs) % cpu.GetReg<s32>(inst.r.rt));
    }
}

void EEInterpreter::mfhi(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<u64>(inst.r.rd, cpu.regs.hi);
}

void EEInterpreter::dsrav(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<s64>(inst.r.rd, cpu.GetReg<s64>(inst.r.rt) >> (cpu.GetReg<u8>(inst.r.rs) & 0x3F));
}

void EEInterpreter::dsll32(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<u64>(inst.r.rd, cpu.GetReg<u64>(inst.r.rt) << (32 + inst.r.sa));
}

void EEInterpreter::dsra32(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<s64>(inst.r.rd, cpu.GetReg<s64>(inst.r.rt) >> (32 + inst.r.sa));
}

void EEInterpreter::movz(EECore& cpu, CPUInstruction inst) {
    if (cpu.GetReg<u64>(inst.r.rt) == 0) {
        cpu.SetReg<u64>(inst.r.rd, cpu.GetReg<u64>(inst.r.rs));
    }
}

void EEInterpreter::dsllv(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<u64>(inst.r.rd, cpu.GetReg<u64>(inst.r.rt) << (cpu.GetReg<u8>(inst.r.rs) & 0x3F));
}

void EEInterpreter::sllv(EECore& cpu, CPUInstruction inst) {
    u32 result = cpu.GetReg<u32>(inst.r.rt) << (cpu.GetReg<u8>(inst.r.rs) & 0x1F);
    cpu.SetReg<s64>(inst.r.rd, sign_extend<s64, 32>(result));
}

void EEInterpreter::dsll(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<u64>(inst.r.rd, cpu.GetReg<u64>(inst.r.rt) << inst.r.sa);
}

void EEInterpreter::srav(EECore& cpu, CPUInstruction inst) {
    s32 result = cpu.GetReg<s32>(inst.r.rt) >> (cpu.GetReg<u8>(inst.r.rs) & 0x1F);
    cpu.SetReg<s64>(inst.r.rd, result);
}

void EEInterpreter::nor(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<u64>(inst.r.rd, ~(cpu.GetReg<u64>(inst.r.rs) | cpu.GetReg<u64>(inst.r.rt)));
}

void EEInterpreter::dsrl(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<u64>(inst.r.rd, cpu.GetReg<u64>(inst.r.rt) >> inst.r.sa);
}

void EEInterpreter::srlv(EECore& cpu, CPUInstruction inst) {
    s32 result = cpu.GetReg<u32>(inst.r.rt) >> (cpu.GetReg<u8>(inst.r.rs) & 0x1F);
    cpu.SetReg<s64>(inst.r.rd, result);
}

void EEInterpreter::dsrl32(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<u64>(inst.r.rd, cpu.GetReg<u64>(inst.r.rt) >> (32 + inst.r.sa));
}

void EEInterpreter::syscall_exception(EECore& cpu, CPUInstruction inst) {
    cpu.DoException(0x80000180, ExceptionType::Syscall);
}

// tlb instructions
void EEInterpreter::tlbwi(EECore& cpu, CPUInstruction inst) {
    // when we handle mmu emulation the tlb will be used
}

void EEInterpreter::di(EECore& cpu, CPUInstruction inst) {
    u32 status = cpu.cop0.GetReg(12);
    bool edi = status & (1 << 17);
    bool exl = status & (1 << 1);
    bool erl = status & (1 << 2);
    bool ksu = ((status >> 3) & 0x3) == 0;

    if (edi || exl || erl || ksu) {
        cpu.cop0.SetReg(12, status & ~(1 << 16));
    } 
}

void EEInterpreter::eret(EECore& cpu, CPUInstruction inst) {
    u32 status = cpu.cop0.GetReg(12);
    bool erl = status & (1 << 2);
    u32 errorepc = cpu.cop0.GetReg(30);
    u32 epc = cpu.cop0.GetReg(14);

    log_warn("eret pc %08x", cpu.regs.pc);

    if (erl) {
        cpu.regs.pc = errorepc - 4;
        cpu.cop0.SetReg(12, status & ~(1 << 2));
    } else {
        cpu.regs.pc = epc - 4;
        cpu.cop0.SetReg(12, status & ~(1 << 1));
    }
}

void EEInterpreter::ei(EECore& cpu, CPUInstruction inst) {
    u32 status = cpu.cop0.GetReg(12);
    bool edi = status & (1 << 17);
    bool exl = status & (1 << 1);
    bool erl = status & (1 << 2);
    bool ksu = ((status >> 3) & 0x3) == 0;

    if (edi || exl || erl || ksu) {
        cpu.cop0.SetReg(12, status | (1 << 16));
    } 
}

void EEInterpreter::COP1Instruction(EECore& cpu, CPUInstruction inst) {
    // TODO: handle fpu instructions
}

void EEInterpreter::COP2Instruction(EECore& cpu, CPUInstruction inst) {
    // TODO: handle vu0 instructions
    // switch (inst.i.rs) {
    // case 2:
    //     cfc2();
    //     break;
    // case 6:
    //     ctc2();
    //     break;
    // default:
    //     log_fatal("handle %d", inst.i.rs);
    //     UndefinedInstruction();
    // }
}

void EEInterpreter::RegImmInstruction(EECore& cpu, CPUInstruction inst) {

}

void EEInterpreter::MMIInstruction(EECore& cpu, CPUInstruction inst) {
    
}

void EEInterpreter::unknown_instruction(EECore& cpu, CPUInstruction inst) {
    log_fatal("%s = %08x at %08x (primary = %d, secondary = %d, regimm = %d) is undefined", EEDisassembleInstruction(inst, cpu.regs.pc).c_str(), inst.data, cpu.regs.pc, inst.i.opcode, inst.r.func, inst.i.rt);
}