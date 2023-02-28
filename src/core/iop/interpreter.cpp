#include "common/log.h"
#include "common/arithmetic.h"
#include "core/iop/interpreter.h"
#include "core/iop/disassembler.h"
#include "core/iop/context.h"

namespace iop {

Interpreter::Interpreter(Context& ctx) : ctx(ctx) {
    for (int i = 0; i < 64; i++) {
        primary_table[i] = &Interpreter::UndefinedInstruction;
        secondary_table[i] = &Interpreter::UndefinedInstruction;
    }

    // primary instructions
    RegisterOpcode(&Interpreter::SecondaryInstruction, 0, InstructionTable::Primary);
    RegisterOpcode(&Interpreter::bcondz, 1, InstructionTable::Primary);
    RegisterOpcode(&Interpreter::j, 2, InstructionTable::Primary);
    RegisterOpcode(&Interpreter::jal, 3, InstructionTable::Primary);
    RegisterOpcode(&Interpreter::beq, 4, InstructionTable::Primary);
    RegisterOpcode(&Interpreter::bne, 5, InstructionTable::Primary);
    RegisterOpcode(&Interpreter::blez, 6, InstructionTable::Primary);
    RegisterOpcode(&Interpreter::bgtz, 7, InstructionTable::Primary);
    RegisterOpcode(&Interpreter::addi, 8, InstructionTable::Primary);
    RegisterOpcode(&Interpreter::addiu, 9, InstructionTable::Primary);
    RegisterOpcode(&Interpreter::slti, 10, InstructionTable::Primary);   
    RegisterOpcode(&Interpreter::sltiu, 11, InstructionTable::Primary); 
    RegisterOpcode(&Interpreter::andi, 12, InstructionTable::Primary);
    RegisterOpcode(&Interpreter::ori, 13, InstructionTable::Primary);
    RegisterOpcode(&Interpreter::lui, 15, InstructionTable::Primary);
    RegisterOpcode(&Interpreter::COP0Instruction, 16, InstructionTable::Primary);
    RegisterOpcode(&Interpreter::lb, 32, InstructionTable::Primary);
    RegisterOpcode(&Interpreter::lh, 33, InstructionTable::Primary);
    RegisterOpcode(&Interpreter::lwl, 34, InstructionTable::Primary);
    RegisterOpcode(&Interpreter::lw, 35, InstructionTable::Primary);
    RegisterOpcode(&Interpreter::lbu, 36, InstructionTable::Primary);
    RegisterOpcode(&Interpreter::lhu, 37, InstructionTable::Primary);
    RegisterOpcode(&Interpreter::lwr, 38, InstructionTable::Primary);
    RegisterOpcode(&Interpreter::sb, 40, InstructionTable::Primary);
    RegisterOpcode(&Interpreter::sh, 41, InstructionTable::Primary);
    RegisterOpcode(&Interpreter::swl, 42, InstructionTable::Primary);
    RegisterOpcode(&Interpreter::sw, 43, InstructionTable::Primary);
    RegisterOpcode(&Interpreter::swr, 46, InstructionTable::Primary);

    // secondary instructions
    RegisterOpcode(&Interpreter::sll, 0, InstructionTable::Secondary);
    RegisterOpcode(&Interpreter::srl, 2, InstructionTable::Secondary);
    RegisterOpcode(&Interpreter::sra, 3, InstructionTable::Secondary);
    RegisterOpcode(&Interpreter::sllv, 4, InstructionTable::Secondary);
    RegisterOpcode(&Interpreter::srlv, 6, InstructionTable::Secondary);
    RegisterOpcode(&Interpreter::srav, 7, InstructionTable::Secondary);
    RegisterOpcode(&Interpreter::jr, 8, InstructionTable::Secondary);
    RegisterOpcode(&Interpreter::jalr, 9, InstructionTable::Secondary);
    RegisterOpcode(&Interpreter::syscall_exception, 12, InstructionTable::Secondary);
    RegisterOpcode(&Interpreter::mfhi, 16, InstructionTable::Secondary);
    RegisterOpcode(&Interpreter::mthi, 17, InstructionTable::Secondary);
    RegisterOpcode(&Interpreter::mflo, 18, InstructionTable::Secondary);
    RegisterOpcode(&Interpreter::mtlo, 19, InstructionTable::Secondary);
    RegisterOpcode(&Interpreter::mult, 24, InstructionTable::Secondary);
    RegisterOpcode(&Interpreter::multu, 25, InstructionTable::Secondary);
    RegisterOpcode(&Interpreter::div, 26, InstructionTable::Secondary);
    RegisterOpcode(&Interpreter::divu, 27, InstructionTable::Secondary);
    RegisterOpcode(&Interpreter::add, 32, InstructionTable::Secondary);
    RegisterOpcode(&Interpreter::addu, 33, InstructionTable::Secondary);
    RegisterOpcode(&Interpreter::subu, 35, InstructionTable::Secondary);
    RegisterOpcode(&Interpreter::andd, 36, InstructionTable::Secondary);
    RegisterOpcode(&Interpreter::orr, 37, InstructionTable::Secondary);
    RegisterOpcode(&Interpreter::xorr, 38, InstructionTable::Secondary);
    RegisterOpcode(&Interpreter::nor, 39, InstructionTable::Secondary);
    RegisterOpcode(&Interpreter::slt, 42, InstructionTable::Secondary);
    RegisterOpcode(&Interpreter::sltu, 43, InstructionTable::Secondary);
}

void Interpreter::Reset() {
    branch_delay = false;
    branch = false;
    inst.data = 0;
}

void Interpreter::Run(int cycles) {
    while (cycles--) {
        inst = Instruction{ctx.ReadWord(ctx.pc)};

        if (ctx.pc == 0x00012c48 || ctx.pc == 0x0001420c || ctx.pc == 0x0001430c) {
            IOPPuts();
        }

        (this->*primary_table[inst.opcode])();

        ctx.pc += 4;

        if (branch_delay) {
            if (branch) {
                ctx.pc = ctx.npc;
                branch_delay = false;
                branch = false;
            } else {
                branch = true;
            }
        }

        CheckInterrupts();
    }
}

void Interpreter::RegisterOpcode(InstructionHandler handler, int index, InstructionTable table) {
    if (table == InstructionTable::Primary) {
        primary_table[index] = handler;
    } else {
        secondary_table[index] = handler;
    }
}

void Interpreter::DoException(Exception exception) {
    common::Log("[IOP] trigger exception with type %02x", static_cast<int>(exception));

    // record the cause of the exception
    ctx.cop0.cause.excode = static_cast<u8>(exception);

    // store pc to epc
    if (branch_delay) {
        ctx.cop0.epc = ctx.pc - 4;
        ctx.cop0.cause.bd = true;
    } else {
        ctx.cop0.epc = ctx.pc;
        ctx.cop0.cause.bd = false;
    }

    if (ctx.cop0.status.bev) {
        common::Log("[iop::Interpreter] handle bev=1");
    }

    u32 target = 0x80000080;

    // shift the interrupt and kernel/user mode bit 
    // by 2 bits to the left to act as a stack with maximum of 3 entries
    u8 stack = ctx.cop0.status.data & 0x3f;
    ctx.cop0.status.data &= ~0x3f;
    ctx.cop0.status.data |= (stack << 2) & 0x3f;

    // since we increment by 4 after each instruction we need to account for that
    // so that we can execute at the exception base on the next instruction
    ctx.pc = target - 4;

    branch_delay = false;
    branch = false;
}

void Interpreter::RaiseInterrupt(bool value) {
    if (value) {
        ctx.cop0.cause.data |= 1 << 10;
    } else {
        ctx.cop0.cause.data &= ~(1 << 10);
    }
}

void Interpreter::CheckInterrupts() {
    if (ctx.cop0.status.iec && (ctx.cop0.status.im & ctx.cop0.cause.ip)) {
        DoException(Exception::Interrupt);
    }
}

void Interpreter::UndefinedInstruction() {
    common::Error("%s %08x at %08x (primary = %d, secondary = %d, regimm = %d) is undefined", DisassembleInstruction(inst, ctx.pc).c_str(), inst.data, ctx.pc, inst.opcode, inst.func, inst.rt);
}

void Interpreter::SecondaryInstruction() {
    (this->*secondary_table[inst.func])();
}

void Interpreter::COP0Instruction() {
    u8 format = inst.rs;

    switch (format) {
    case 0:
        mfc0();
        break;
    case 4:
        mtc0();
        break;
    case 16:
        rfe();
        break;
    default:
        common::Error("handle %d", format);
    }
}

void Interpreter::IOPPuts() {
    u32 address = ctx.GetReg(5);
    u32 length = ctx.GetReg(6);
    
    // for (u32 i = 0; i < length; i++) {
        // common::LogNoNewline("%c", system->memory.iop_ram[address & 0x1fffff]);
        // address++;
    // }
}

// primary
void Interpreter::slti() {
    ctx.SetReg(inst.rt, (s32)ctx.GetReg(inst.rs) < SignExtend<s32, 16>(inst.imm));
}

void Interpreter::bne() {
    s32 offset = SignExtend<s32, 16>(inst.imm) << 2;

    if (ctx.GetReg(inst.rs) != ctx.GetReg(inst.rt)) {
        ctx.npc = ctx.pc + offset + 4;
        branch_delay = true;
    }
}

void Interpreter::lui() {
    ctx.SetReg(inst.rt, inst.imm << 16);
}

void Interpreter::ori() {
    ctx.SetReg(inst.rt, ctx.GetReg(inst.rs) | inst.imm);
}

void Interpreter::beq() {
    s32 offset = SignExtend<s32, 16>(inst.imm) << 2;

    if (ctx.GetReg(inst.rs) == ctx.GetReg(inst.rt)) {
        ctx.npc = ctx.pc + offset + 4;
        branch_delay = true;
    }
}

void Interpreter::lw() {
    if (!ctx.cop0.status.isc) {
        ctx.SetReg(inst.rt, ctx.ReadWord(ctx.GetReg(inst.rs) + SignExtend<s32, 16>(inst.imm)));
    }
}

void Interpreter::andi() {
    ctx.SetReg(inst.rt, ctx.GetReg(inst.rs) & inst.imm);
}

void Interpreter::addiu() {
    ctx.SetReg(inst.rt, ctx.GetReg(inst.rs) + SignExtend<s32, 16>(inst.imm));
}

void Interpreter::addi() {
    ctx.SetReg(inst.rt, ctx.GetReg(inst.rs) + SignExtend<s32, 16>(inst.imm));
}

void Interpreter::sw() {
    if (!ctx.cop0.status.isc) {
        ctx.WriteWord(ctx.GetReg(inst.rs) + SignExtend<s32, 16>(inst.imm), ctx.GetReg(inst.rt));
    }
}

void Interpreter::sb() {
    if (!ctx.cop0.status.isc) {
        ctx.WriteByte(ctx.GetReg(inst.rs) + SignExtend<s32, 16>(inst.imm), ctx.GetReg(inst.rt));
    }
}

void Interpreter::lb() {
    if (!ctx.cop0.status.isc) {
        ctx.SetReg(inst.rt, SignExtend<s32, 8>(ctx.ReadByte(ctx.GetReg(inst.rs) + SignExtend<s32, 16>(inst.imm))));
    }
}

void Interpreter::jal() {
    ctx.SetReg(31, ctx.pc + 8);
    ctx.npc = (ctx.pc & 0xF0000000) + (inst.offset << 2);
    branch_delay = true;
}

void Interpreter::lh() {
    if (!ctx.cop0.status.isc) {
        u32 addr = ctx.gpr[inst.rs] + SignExtend<s32, 16>(inst.imm);

        ctx.SetReg(inst.rt, SignExtend<s32, 16>(ctx.ReadHalf(addr)));
    }
}

void Interpreter::j() {
    ctx.npc = (ctx.pc & 0xF0000000) + (inst.offset << 2);
    branch_delay = true;
}

void Interpreter::lbu() {
    if (!ctx.cop0.status.isc) {
        ctx.SetReg(inst.rt, ctx.ReadByte(ctx.GetReg(inst.rs) + SignExtend<s32, 16>(inst.imm)));
    }
}

void Interpreter::sltiu() {
    ctx.SetReg(inst.rt, ctx.GetReg(inst.rs) < (u32)SignExtend<s32, 16>(inst.imm));
}

void Interpreter::lhu() {
    if (!ctx.cop0.status.isc) {
        u32 addr = ctx.GetReg(inst.rs) + SignExtend<s32, 16>(inst.imm);

        ctx.SetReg(inst.rt, ctx.ReadHalf(addr));
    }
}

void Interpreter::blez() {
    if (static_cast<s32>(ctx.GetReg(inst.rs)) <= 0) {
        ctx.npc = ctx.pc + (SignExtend<s32, 16>(inst.imm) << 2) + 4;
        branch_delay = true;
    }
}

void Interpreter::bgtz() {
    if (static_cast<s32>(ctx.GetReg(inst.rs)) > 0) {
        ctx.npc = ctx.pc + (SignExtend<s32, 16>(inst.imm) << 2) + 4;
        branch_delay = true;
    }
}

void Interpreter::sh() {
    if (!ctx.cop0.status.isc) {
        u32 addr = ctx.GetReg(inst.rs) + SignExtend<s32, 16>(inst.imm);
        ctx.WriteHalf(addr, ctx.GetReg(inst.rt));
    }
}

void Interpreter::bcondz() {
    bool link = (inst.rt & 0x1E) == 0x10;
    bool ge = inst.rt & 0x1;
    bool branch = (static_cast<s32>(ctx.GetReg(inst.rs)) < 0) ^ ge;

    if (branch) {
        ctx.npc = ctx.pc + (SignExtend<s32, 16>(inst.imm) << 2) + 4;
        branch_delay = true;
    }

    if (link) {
        ctx.SetReg(31, ctx.pc + 8);
    }
}

void Interpreter::lwl() {
    u32 addr = ctx.GetReg(inst.rs) + inst.simm;
    u32 aligned_data = ctx.ReadWord(addr & ~0x3);
    u8 shift = (addr & 0x3) * 8;
    u32 mask = 0xFFFFFF >> shift;
    
    ctx.SetReg(inst.rt, (ctx.GetReg(inst.rt) & mask) | (aligned_data << (24 - shift)));
}

void Interpreter::lwr() {
    u32 addr = ctx.GetReg(inst.rs) + inst.simm;
    u32 aligned_data = ctx.ReadWord(addr & ~0x3);
    u8 shift = (addr & 0x3) * 8;
    u32 mask = 0xFFFFFF00 << (24 - shift);
    
    ctx.SetReg(inst.rt, (ctx.GetReg(inst.rt) & mask) | (aligned_data >> shift));
}

void Interpreter::swl() {
    u32 addr = ctx.GetReg(inst.rs) + inst.simm;
    u32 aligned_data = ctx.ReadWord(addr & ~0x3);
    u8 shift = (addr & 0x3) * 8;
    u32 mask = 0xFFFFFF00 << shift;
    u32 updated_data = (aligned_data & mask) | (ctx.GetReg(inst.rt) >> (24 - shift));

    ctx.WriteWord(addr & ~0x3, updated_data);
}

void Interpreter::swr() {
    u32 addr = ctx.GetReg(inst.rs) + inst.simm;
    u32 aligned_data = ctx.ReadWord(addr & ~0x3);
    u8 shift = (addr & 0x3) * 8;
    u32 mask = 0xFFFFFF >> (24 - shift);
    u32 updated_data = (aligned_data & mask) | (ctx.GetReg(inst.rt) << shift);

    ctx.WriteWord(addr & ~0x3, updated_data);
}

// secondary
void Interpreter::sll() {
    ctx.SetReg(inst.rd, ctx.GetReg(inst.rt) << inst.imm5);
}

void Interpreter::jr() {
    ctx.npc = ctx.GetReg(inst.rs);
    branch_delay = true;
}

void Interpreter::orr() {
    ctx.SetReg(inst.rd, ctx.GetReg(inst.rs) | ctx.GetReg(inst.rt));
}

void Interpreter::addu() {
    ctx.SetReg(inst.rd, ctx.GetReg(inst.rs) + ctx.GetReg(inst.rt));
}

void Interpreter::sltu() {
    ctx.SetReg(inst.rd, ctx.GetReg(inst.rs) < ctx.GetReg(inst.rt));
}

void Interpreter::andd() {
    ctx.SetReg(inst.rd, ctx.GetReg(inst.rs) & ctx.GetReg(inst.rt));
}

void Interpreter::slt() {
    ctx.SetReg(inst.rd, (s32)ctx.GetReg(inst.rs) < (s32)ctx.GetReg(inst.rt));
}

void Interpreter::sra() {
    ctx.SetReg(inst.rd, (s32)ctx.GetReg(inst.rt) >> inst.imm5);
}

void Interpreter::srl() {
    ctx.SetReg(inst.rd, ctx.GetReg(inst.rt) >> inst.imm5);
}

void Interpreter::subu() {
    ctx.SetReg(inst.rd, ctx.GetReg(inst.rs) - ctx.GetReg(inst.rt));
}

void Interpreter::divu() {
    if (ctx.GetReg(inst.rt) == 0) {
        ctx.lo = 0xFFFFFFFF;
        ctx.hi = ctx.GetReg(inst.rs);
    } else {
        ctx.lo = ctx.GetReg(inst.rs) / ctx.GetReg(inst.rt);
        ctx.hi = ctx.GetReg(inst.rs) % ctx.GetReg(inst.rt);
    }
}

void Interpreter::mflo() {
    ctx.SetReg(inst.rd, ctx.lo);
}

void Interpreter::jalr() {
    u32 addr = ctx.GetReg(inst.rs);

    ctx.SetReg(inst.rd, ctx.pc + 8);
    
    ctx.npc = addr;
    branch_delay = true;
}

void Interpreter::xorr() {
    ctx.SetReg(inst.rd, ctx.GetReg(inst.rs) ^ ctx.GetReg(inst.rt));
}

void Interpreter::sllv() {
    ctx.SetReg(inst.rd, ctx.GetReg(inst.rt) << (ctx.GetReg(inst.rs) & 0x1F));
}

void Interpreter::mfhi() {
    ctx.SetReg(inst.rd, ctx.hi);
}

void Interpreter::multu() {
    u64 result = (u64)ctx.GetReg(inst.rs) * (u64)ctx.GetReg(inst.rt);

    ctx.lo = result & 0xFFFFFFFF;
    ctx.hi = result >> 32;
}

void Interpreter::mthi() {
    ctx.hi = ctx.GetReg(inst.rs);
}

void Interpreter::mtlo() {
    ctx.lo = ctx.GetReg(inst.rs);
}

void Interpreter::syscall_exception() {
    DoException(Exception::Syscall);
}

void Interpreter::mult() {
    s64 result = (s64)(s32)ctx.GetReg(inst.rs) * (s64)(s32)ctx.GetReg(inst.rt);

    ctx.lo = result & 0xFFFFFFFF;
    ctx.hi = result >> 32;
}

void Interpreter::nor() {
    ctx.SetReg(inst.rd, 0xFFFFFFFF ^ (ctx.GetReg(inst.rs) | ctx.GetReg(inst.rt)));
}

void Interpreter::srlv() {
    ctx.SetReg(inst.rd, ctx.GetReg(inst.rt) >> (ctx.GetReg(inst.rs) & 0x1F));
}

void Interpreter::add() {
    ctx.SetReg(inst.rd, ctx.GetReg(inst.rs) + ctx.GetReg(inst.rt));
}

void Interpreter::div() {
    s32 rs = (s32)ctx.GetReg(inst.rs);
    s32 rt = (s32)ctx.GetReg(inst.rt);

    if (rt == 0) {
        if (rs < 0) {
            ctx.lo = 1;
        } else {
            ctx.lo = 0xFFFFFFFF;
        }

        ctx.hi = rs;
    } else if ((u32)rs == 0x80000000 && (u32)rt == 0xFFFFFFFF) {
        ctx.lo = 0x80000000;
        ctx.hi = 0;
    } else {
        ctx.lo = rs / rt;
        ctx.hi = rs % rt;
    }
}

void Interpreter::srav() {
    u8 shift_amount = ctx.GetReg(inst.rs) & 0x1F;
    ctx.SetReg(inst.rd, (s32)ctx.GetReg(inst.rt) >> shift_amount);
}

// cop0 instructions
void Interpreter::mfc0() {
    ctx.SetReg(inst.rt, ctx.cop0.GetReg(inst.rd));
}

void Interpreter::mtc0() {
    ctx.cop0.SetReg(inst.rd, ctx.GetReg(inst.rt));
}

void Interpreter::rfe() {
    u8 stack = ctx.cop0.status.data & 0x3f;
    ctx.cop0.status.data &= ~0xf;
    ctx.cop0.status.data |= stack >> 2;
}

} // namespace iop