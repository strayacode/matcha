#include "common/types.h"
#include "common/log_file.h"
#include "common/arithmetic.h"
#include "core/ee/ee_interpreter.h"
#include "core/ee/disassembler.h"
#include "core/system.h"

// COP0 instructions
void EEInterpreter::mfc0(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<s64>(inst.rt, static_cast<s32>(cpu.cop0.GetReg(inst.rd)));
}

void EEInterpreter::mtc0(EECore& cpu, CPUInstruction inst) {
    cpu.cop0.SetReg(inst.rd, cpu.GetReg<u32>(inst.rt));
}

// COP1 instructions
void EEInterpreter::swc1(EECore& cpu, CPUInstruction inst) {
    cpu.WriteWord(cpu.GetReg<u32>(inst.rs) + inst.simm, cpu.cop1.GetReg(inst.rt));
}

void EEInterpreter::mtc1(EECore& cpu, CPUInstruction inst) {
    cpu.cop1.SetReg(inst.rd, cpu.GetReg<u32>(inst.rt));
}

void EEInterpreter::adda_s(EECore& cpu, CPUInstruction inst) {
    cpu.cop1.accumulator.f = cpu.cop1.AsFloat(cpu.cop1.GetReg(inst.rd)) + cpu.cop1.AsFloat(cpu.cop1.GetReg(inst.rt));

    // TODO: handle overflows and underflows
}

void EEInterpreter::ctc1(EECore& cpu, CPUInstruction inst) {
    cpu.cop1.SetControlReg(inst.rd, cpu.GetReg<u32>(inst.rt));
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
    if (cpu.GetReg<u32>(inst.rt)) {
        cpu.lo1 = sign_extend<s64, 32>(cpu.GetReg<u32>(inst.rs) / cpu.GetReg<u32>(inst.rt));
        cpu.hi1 = sign_extend<s64, 32>(cpu.GetReg<u32>(inst.rs) % cpu.GetReg<u32>(inst.rt));
    } else {
        cpu.lo1 = 0xFFFFFFFFFFFFFFFF;
        cpu.hi1 = sign_extend<s64, 32>(cpu.GetReg<u32>(inst.rs));
    }
}

void EEInterpreter::mflo1(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<u64>(inst.rd, cpu.lo1);
}

void EEInterpreter::mult1(EECore& cpu, CPUInstruction inst) {
    s64 result = cpu.GetReg<s32>(inst.rt) * cpu.GetReg<s32>(inst.rs);
    cpu.lo1 = sign_extend<s64, 32>(result & 0xFFFFFFFF);
    cpu.hi1 = sign_extend<s64, 32>(result >> 32);
    cpu.SetReg<u64>(inst.rd, cpu.lo1);
}

void EEInterpreter::por(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<u128>(inst.rd, cpu.GetReg<u128>(inst.rs) | cpu.GetReg<u128>(inst.rt));
}

void EEInterpreter::padduw(EECore& cpu, CPUInstruction inst) {
    for (int i = 0; i < 4; i++) {
        u64 result = cpu.GetReg<u32>(inst.rs, i) + cpu.GetReg<u32>(inst.rt, i);

        if (result > 0xFFFFFFFF) {
            cpu.SetReg<u32>(inst.rd, 0xFFFFFFFF, i);
        } else {
            cpu.SetReg<u32>(inst.rd, result);
        }
    }
}

void EEInterpreter::plzcw(EECore& cpu, CPUInstruction inst) {
    u32 words[2];
    u32 results[2];
    words[0] = cpu.GetReg<u32>(inst.rs, 0);
    words[1] = cpu.GetReg<u32>(inst.rs, 1);

    for (int i = 0; i < 2; i++) {
        bool leading_bit = (words[i] >> 31) & 0x1;
        
        for (int bit = 30; bit >= 0; bit--) {
            if (((words[i] >> bit) & 0x1) == leading_bit) {
                results[i]++;
            } else {
                break;
            }
        }

        cpu.SetReg<u32>(inst.rd, words[i], i);
    }
}

// primary instructions
void EEInterpreter::slti(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<u64>(inst.rt, cpu.GetReg<s64>(inst.rs) < sign_extend<s64, 16>(inst.imm));
}

void EEInterpreter::bne(EECore& cpu, CPUInstruction inst) {
    s32 offset = inst.simm << 2;

    if (cpu.GetReg<u64>(inst.rs) != cpu.GetReg<u64>(inst.rt)) {
        cpu.next_pc = cpu.pc + offset + 4;
        cpu.branch_delay = true;
    }
}

void EEInterpreter::lui(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<u64>(inst.rt, sign_extend<s64, 32>(inst.imm << 16));
}

void EEInterpreter::ori(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<u64>(inst.rt, cpu.GetReg<u64>(inst.rs) | inst.imm);
}

void EEInterpreter::addiu(EECore& cpu, CPUInstruction inst) {
    s32 result = cpu.GetReg<s64>(inst.rs) + (s16)inst.imm;
    cpu.SetReg<s64>(inst.rt, result);
}

void EEInterpreter::sw(EECore& cpu, CPUInstruction inst) {
    cpu.WriteWord(cpu.GetReg<u32>(inst.rs) + inst.simm, cpu.GetReg<u32>(inst.rt));
}

void EEInterpreter::sd(EECore& cpu, CPUInstruction inst) {
    cpu.WriteDouble(cpu.GetReg<u32>(inst.rs) + inst.simm, cpu.GetReg<u64>(inst.rt));
}

void EEInterpreter::jal(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<u64>(31, cpu.pc + 8);
    cpu.next_pc = ((cpu.pc + 4) & 0xF0000000) + (inst.offset << 2);
    cpu.branch_delay = true;
}

void EEInterpreter::andi(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<u64>(inst.rt, cpu.GetReg<u64>(inst.rs) & inst.imm);
}

void EEInterpreter::beq(EECore& cpu, CPUInstruction inst) {
    s32 offset = inst.simm << 2;

    if (cpu.GetReg<u64>(inst.rs) == cpu.GetReg<u64>(inst.rt)) {
        cpu.next_pc = cpu.pc + offset + 4;
        cpu.branch_delay = true;
    }
}

void EEInterpreter::beql(EECore& cpu, CPUInstruction inst) {
    s32 offset = inst.simm << 2;

    if (cpu.GetReg<u64>(inst.rs) == cpu.GetReg<u64>(inst.rt)) {
        cpu.next_pc = cpu.pc + offset + 4;
        cpu.branch_delay = true;
    } else {
        cpu.pc += 4;
    }
}

void EEInterpreter::sltiu(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<u64>(inst.rt, cpu.GetReg<u64>(inst.rs) < (u64)(s64)inst.simm);
}

void EEInterpreter::bnel(EECore& cpu, CPUInstruction inst) {
    s32 offset = inst.simm << 2;

    if (cpu.GetReg<u64>(inst.rs) != cpu.GetReg<u64>(inst.rt)) {
        cpu.next_pc = cpu.pc + offset + 4;
        cpu.branch_delay = true;
    } else {
        cpu.pc += 4;
    }
}

void EEInterpreter::lb(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<s64>(inst.rt, (s8)cpu.ReadByte(cpu.GetReg<u32>(inst.rs) + inst.simm));
}

void EEInterpreter::lbu(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<u64>(inst.rt, cpu.ReadByte(cpu.GetReg<u32>(inst.rs) + inst.simm));
}

void EEInterpreter::ld(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<u64>(inst.rt, cpu.ReadDouble(cpu.GetReg<u32>(inst.rs) + inst.simm));
}

void EEInterpreter::j(EECore& cpu, CPUInstruction inst) {
    cpu.next_pc = ((cpu.pc + 4) & 0xF0000000) + (inst.offset << 2);
    cpu.branch_delay = true;
}

void EEInterpreter::lw(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<s64>(inst.rt, (s32)cpu.ReadWord(cpu.GetReg<u32>(inst.rs) + inst.simm));
}

void EEInterpreter::sb(EECore& cpu, CPUInstruction inst) {
    cpu.WriteByte(cpu.GetReg<u32>(inst.rs) + inst.simm, cpu.GetReg<u8>(inst.rt));
}

void EEInterpreter::blez(EECore& cpu, CPUInstruction inst) {
    s32 offset = inst.simm << 2;

    if (cpu.GetReg<s64>(inst.rs) <= 0) {
        cpu.next_pc = cpu.pc + offset + 4;
        cpu.branch_delay = true;
    }
}

void EEInterpreter::lhu(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<u64>(inst.rt, cpu.ReadHalf(cpu.GetReg<u32>(inst.rs) + inst.simm));
}

void EEInterpreter::bgtz(EECore& cpu, CPUInstruction inst) {
    s32 offset = inst.simm << 2;

    if (cpu.GetReg<s64>(inst.rs) > 0) {
        cpu.next_pc = cpu.pc + offset + 4;
        cpu.branch_delay = true;
    }
}

void EEInterpreter::sh(EECore& cpu, CPUInstruction inst) {
    cpu.WriteHalf(cpu.GetReg<u32>(inst.rs) + inst.simm, cpu.GetReg<u16>(inst.rt));
}

void EEInterpreter::xori(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<u64>(inst.rt, cpu.GetReg<u64>(inst.rs) ^ inst.imm);
}

void EEInterpreter::daddiu(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<s64>(inst.rt, cpu.GetReg<s64>(inst.rs) + sign_extend<s64, 16>(inst.imm));
}

void EEInterpreter::sq(EECore& cpu, CPUInstruction inst) {
    u128 reg = cpu.GetReg<u128>(inst.rt);
    u32 addr = (cpu.GetReg<u32>(inst.rs) + inst.simm) & ~0xF;

    cpu.WriteQuad(addr, reg);
}

void EEInterpreter::lq(EECore& cpu, CPUInstruction inst) {
    u128 data;
    u32 addr = (cpu.GetReg<u32>(inst.rs) + inst.simm) & ~0xF;

    data.i.lo = cpu.ReadDouble(addr);
    data.i.hi = cpu.ReadDouble(addr + 8);

    cpu.SetReg<u128>(inst.rt, data);
}

void EEInterpreter::lh(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<s64>(inst.rt, (s16)cpu.ReadHalf(cpu.GetReg<u32>(inst.rs) + inst.simm));
}

void EEInterpreter::cache(EECore& cpu, CPUInstruction inst) {
    // handle later
}

void EEInterpreter::lwu(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<u64>(inst.rt, cpu.ReadWord(cpu.GetReg<u32>(inst.rs) + inst.simm));
}

void EEInterpreter::ldl(EECore& cpu, CPUInstruction inst) {
    const u64 mask[8] = {
        0x00FFFFFFFFFFFFFF, 0x0000FFFFFFFFFFFF, 0x000000FFFFFFFFFF, 0x00000000FFFFFFFF,
        0x0000000000FFFFFF, 0x000000000000FFFF, 0x00000000000000FF, 0x0000000000000000,
    };

    const u8 shift[8] = {56, 48, 40, 32, 24, 16, 8, 0};

    u32 addr = cpu.GetReg<u32>(inst.rs) + inst.simm;
    int index = addr & 0x7;
    u64 data = cpu.ReadDouble(addr & ~0x7);
    u64 reg = cpu.GetReg<u64>(inst.rt);
    cpu.SetReg<u64>(inst.rt, (reg & mask[index]) | (data << shift[index]));
}

void EEInterpreter::ldr(EECore& cpu, CPUInstruction inst) {
    const u64 mask[8] = {
        0x0000000000000000, 0xFF00000000000000, 0xFFFF000000000000, 0xFFFFFF0000000000,
        0xFFFFFFFF00000000, 0xFFFFFFFFFF000000, 0xFFFFFFFFFFFF0000, 0xFFFFFFFFFFFFFF00,
    };

    const u8 shift[8] = {0, 8, 16, 24, 32, 40, 48, 56};

    u32 addr = cpu.GetReg<u32>(inst.rs) + inst.simm;
    int index = addr & 0x7;
    u64 data = cpu.ReadDouble(addr & ~0x7);
    u64 reg = cpu.GetReg<u64>(inst.rt);
    cpu.SetReg<u64>(inst.rt, (reg & mask[index]) | (data >> shift[index]));
}

void EEInterpreter::sdl(EECore& cpu, CPUInstruction inst) {
    const u64 mask[8] = {
        0xFFFFFFFFFFFFFF00, 0xFFFFFFFFFFFF0000, 0xFFFFFFFFFF000000, 0xFFFFFFFF00000000,
        0xFFFFFF0000000000, 0xFFFF000000000000, 0xFF00000000000000, 0x0000000000000000,
    };

    const u8 shift[8] = {56, 48, 40, 32, 24, 16, 8, 0};

    u32 addr = cpu.GetReg<u32>(inst.rs) + inst.simm;
    int index = addr & 0x7;

    u64 data = cpu.ReadDouble(addr & ~0x7);
    u64 reg = cpu.GetReg<u64>(inst.rt);
    cpu.WriteDouble(addr & ~0x7, (reg >> shift[index]) | (data & mask[index]));
}

void EEInterpreter::sdr(EECore& cpu, CPUInstruction inst) {
    const u64 mask[8] = {
        0x0000000000000000, 0x00000000000000ff, 0x000000000000ffff, 0x0000000000ffffff,
        0x00000000ffffffff, 0x000000ffffffffff, 0x0000ffffffffffff, 0x00ffffffffffffff,
    };

    const u8 shift[8] = {0, 8, 16, 24, 32, 40, 48, 56};

    u32 addr = cpu.GetReg<u32>(inst.rs) + inst.simm;
    int index = addr & 0x7;

    u64 data = cpu.ReadDouble(addr & ~0x7);
    u64 reg = cpu.GetReg<u64>(inst.rt);
    cpu.WriteDouble(addr & ~0x7, (reg << shift[index]) | (data & mask[index]));
}

// regimm instructions
void EEInterpreter::bgez(EECore& cpu, CPUInstruction inst) {
    s32 offset = inst.simm << 2;

    if (cpu.GetReg<s64>(inst.rs) >= 0) {
        cpu.next_pc = cpu.pc + offset + 4;
        cpu.branch_delay = true;
    }
}

void EEInterpreter::bltz(EECore& cpu, CPUInstruction inst) {
    s32 offset = inst.simm << 2;

    if (cpu.GetReg<s64>(inst.rs) < 0) {
        cpu.next_pc = cpu.pc + offset + 4;
        cpu.branch_delay = true;
    }
}

void EEInterpreter::bltzl(EECore& cpu, CPUInstruction inst) {
    s32 offset = inst.simm << 2;

    if (cpu.GetReg<s64>(inst.rs) < 0) {
        cpu.next_pc = cpu.pc + offset + 4;
        cpu.branch_delay = true;
    } else {
        cpu.pc += 4;
    }
}

void EEInterpreter::bgezl(EECore& cpu, CPUInstruction inst) {
    s32 offset = inst.simm << 2;

    if (cpu.GetReg<s64>(inst.rs) >= 0) {
        cpu.next_pc = cpu.pc + offset + 4;
        cpu.branch_delay = true;
    } else {
        cpu.pc += 4;
    }
}

// secondary instructions
void EEInterpreter::sll(EECore& cpu, CPUInstruction inst) {
    u32 result = cpu.GetReg<u32>(inst.rt) << inst.imm5;
    cpu.SetReg<s64>(inst.rd, (s32)result);
}

void EEInterpreter::jr(EECore& cpu, CPUInstruction inst) {
    cpu.next_pc = cpu.GetReg<u32>(inst.rs);
    cpu.branch_delay = true;
}

void EEInterpreter::sync(EECore& cpu, CPUInstruction inst) {
    
}

void EEInterpreter::jalr(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<u64>(inst.rd, cpu.pc + 8);
    cpu.next_pc = cpu.GetReg<u32>(inst.rs);
    cpu.branch_delay = true;
}

void EEInterpreter::daddu(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<u64>(inst.rd, cpu.GetReg<s64>(inst.rs) + cpu.GetReg<s64>(inst.rt));
}

void EEInterpreter::orr(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<u64>(inst.rd, cpu.GetReg<u64>(inst.rs) | cpu.GetReg<u64>(inst.rt));
}

void EEInterpreter::mult(EECore& cpu, CPUInstruction inst) {
    s64 result = cpu.GetReg<s32>(inst.rs) * cpu.GetReg<s32>(inst.rt);
    cpu.lo = (s32)(result & 0xFFFFFFFF);
    cpu.hi = (s32)(result >> 32);
    cpu.SetReg<u64>(inst.rd, cpu.lo);
}

void EEInterpreter::divu(EECore& cpu, CPUInstruction inst) {
    if (cpu.GetReg<u32>(inst.rt) == 0) {
        cpu.lo = 0xFFFFFFFFFFFFFFFF;
        cpu.hi = sign_extend<s64, 32>(cpu.GetReg<u32>(inst.rs));
    } else {
        cpu.lo = sign_extend<s64, 32>(cpu.GetReg<u32>(inst.rs) / cpu.GetReg<u32>(inst.rt));
        cpu.hi = sign_extend<s64, 32>(cpu.GetReg<u32>(inst.rs) % cpu.GetReg<u32>(inst.rt));
    }
}

void EEInterpreter::break_exception(EECore& cpu, CPUInstruction inst) {
    // not sure what to do here
}

void EEInterpreter::mflo(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<u64>(inst.rd, cpu.lo);
}

void EEInterpreter::srl(EECore& cpu, CPUInstruction inst) {
    u32 result = cpu.GetReg<u32>(inst.rt) >> inst.imm5;
    cpu.SetReg<s64>(inst.rd, (s32)result);
}

void EEInterpreter::sra(EECore& cpu, CPUInstruction inst) {
    s32 result = cpu.GetReg<s32>(inst.rt) >> inst.imm5;
    cpu.SetReg<s64>(inst.rd, result);
}

void EEInterpreter::slt(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<u64>(inst.rd, cpu.GetReg<s64>(inst.rs) < cpu.GetReg<s64>(inst.rt));
}

void EEInterpreter::addu(EECore& cpu, CPUInstruction inst) {
    s32 result = cpu.GetReg<s64>(inst.rs) + cpu.GetReg<s64>(inst.rt);
    cpu.SetReg<s64>(inst.rd, result);
}

void EEInterpreter::sltu(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<u64>(inst.rd, cpu.GetReg<u64>(inst.rs) < cpu.GetReg<u64>(inst.rt));
}

void EEInterpreter::andd(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<u64>(inst.rd, cpu.GetReg<u64>(inst.rs) & cpu.GetReg<u64>(inst.rt));
}

void EEInterpreter::movn(EECore& cpu, CPUInstruction inst) {
    if (cpu.GetReg<u64>(inst.rt)) {
        cpu.SetReg<u64>(inst.rd, cpu.GetReg<u64>(inst.rs));
    }
}

void EEInterpreter::subu(EECore& cpu, CPUInstruction inst) {
    s32 result = cpu.GetReg<s32>(inst.rs) - cpu.GetReg<s32>(inst.rt);
    cpu.SetReg<s64>(inst.rd, result);
}

void EEInterpreter::div(EECore& cpu, CPUInstruction inst) {
    if ((cpu.GetReg<s32>(inst.rs) == (s32)0x80000000) && (cpu.GetReg<s32>(inst.rt) == -1)) {
        cpu.lo = 0x80000000;
        cpu.hi = 0;
    } else if (cpu.GetReg<s32>(inst.rt) == 0) {
        log_fatal("can't divide by 0");
    } else {
        cpu.lo = sign_extend<s64, 32>(cpu.GetReg<s32>(inst.rs) / cpu.GetReg<s32>(inst.rt));
        cpu.hi = sign_extend<s64, 32>(cpu.GetReg<s32>(inst.rs) % cpu.GetReg<s32>(inst.rt));
    }
}

void EEInterpreter::mfhi(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<u64>(inst.rd, cpu.hi);
}

void EEInterpreter::dsrav(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<s64>(inst.rd, cpu.GetReg<s64>(inst.rt) >> (cpu.GetReg<u8>(inst.rs) & 0x3F));
}

void EEInterpreter::dsll32(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<u64>(inst.rd, cpu.GetReg<u64>(inst.rt) << (32 + inst.imm5));
}

void EEInterpreter::dsra32(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<s64>(inst.rd, cpu.GetReg<s64>(inst.rt) >> (32 + inst.imm5));
}

void EEInterpreter::movz(EECore& cpu, CPUInstruction inst) {
    if (cpu.GetReg<u64>(inst.rt) == 0) {
        cpu.SetReg<u64>(inst.rd, cpu.GetReg<u64>(inst.rs));
    }
}

void EEInterpreter::dsllv(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<u64>(inst.rd, cpu.GetReg<u64>(inst.rt) << (cpu.GetReg<u8>(inst.rs) & 0x3F));
}

void EEInterpreter::sllv(EECore& cpu, CPUInstruction inst) {
    u32 result = cpu.GetReg<u32>(inst.rt) << (cpu.GetReg<u8>(inst.rs) & 0x1F);
    cpu.SetReg<s64>(inst.rd, sign_extend<s64, 32>(result));
}

void EEInterpreter::dsll(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<u64>(inst.rd, cpu.GetReg<u64>(inst.rt) << inst.imm5);
}

void EEInterpreter::srav(EECore& cpu, CPUInstruction inst) {
    s32 result = cpu.GetReg<s32>(inst.rt) >> (cpu.GetReg<u8>(inst.rs) & 0x1F);
    cpu.SetReg<s64>(inst.rd, result);
}

void EEInterpreter::nor(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<u64>(inst.rd, ~(cpu.GetReg<u64>(inst.rs) | cpu.GetReg<u64>(inst.rt)));
}

void EEInterpreter::dsrl(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<u64>(inst.rd, cpu.GetReg<u64>(inst.rt) >> inst.imm5);
}

void EEInterpreter::srlv(EECore& cpu, CPUInstruction inst) {
    s32 result = cpu.GetReg<u32>(inst.rt) >> (cpu.GetReg<u8>(inst.rs) & 0x1F);
    cpu.SetReg<s64>(inst.rd, result);
}

void EEInterpreter::dsrl32(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<u64>(inst.rd, cpu.GetReg<u64>(inst.rt) >> (32 + inst.imm5));
}

void EEInterpreter::syscall_exception(EECore& cpu, CPUInstruction inst) {
    u8 opcode = cpu.ReadByte(cpu.pc - 4);

    LogFile::Get().Log("[EE] executing syscall %s\n", cpu.GetSyscallInfo(opcode).c_str());
    cpu.DoException(0x80000180, ExceptionType::Syscall);
}

void EEInterpreter::dsubu(EECore& cpu, CPUInstruction inst) {
    cpu.SetReg<u64>(inst.rd, cpu.GetReg<s64>(inst.rs) - cpu.GetReg<s64>(inst.rt));
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

    if (erl) {
        cpu.pc = errorepc - 4;
        cpu.cop0.SetReg(12, status & ~(1 << 2));
    } else {
        cpu.pc = epc - 4;
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

void EEInterpreter::unknown_instruction(EECore& cpu, CPUInstruction inst) {
    log_fatal("%s = %08x at %08x (primary = %d, secondary = %d, regimm = %d, fmt = %d, imm5 = %d) is undefined", EEDisassembleInstruction(inst, cpu.pc).c_str(), inst.data, cpu.pc, inst.opcode, inst.func, inst.rt, inst.rs, inst.imm5);
}

void EEInterpreter::stub_instruction(EECore& cpu, CPUInstruction inst) {
    // log_warn("%s = %08x at %08x (primary = %d, secondary = %d, regimm = %d) is undefined", EEDisassembleInstruction(inst, cpu.pc).c_str(), inst.data, cpu.pc, inst.opcode, inst.func, inst.rt);
}