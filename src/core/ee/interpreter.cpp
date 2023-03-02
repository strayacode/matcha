#include "common/types.h"
#include "common/log.h"
#include "common/arithmetic.h"
#include "core/ee/context.h"
#include "core/ee/interpreter.h"
#include "core/ee/disassembler.h"
#include "core/system.h"

namespace ee {

Interpreter::Interpreter(Context& ctx) : ctx(ctx) {}

void Interpreter::Reset() {
    branch_delay = false;
    branch = false;
    inst.data = 0;
}

void Interpreter::Run(int cycles) {
    while (cycles--) {
        inst = Instruction{ctx.Read<u32>(ctx.pc)};
        auto handler = decoder.GetHandler(inst);
        (this->*handler)();
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

        ctx.cop0.CountUp();
        CheckInterrupts();
    }
}

void Interpreter::DoException(u32 target, ExceptionType exception) {
    common::Log("[ee::Interpreter] trigger exception with type %02x at pc = %08x", static_cast<int>(exception), ctx.pc);

    bool level2_exception = static_cast<int>(exception) >= 14;
    int code = level2_exception ? static_cast<int>(exception) - 14 : static_cast<int>(exception);

    if (level2_exception) {
        common::Error("[ee::Interpreter] handle level 2 exception");
    } else {
        ctx.cop0.cause.exception = code;
        ctx.cop0.gpr[14] = ctx.pc - 4 * branch_delay;
        ctx.cop0.cause.bd = branch_delay;
        ctx.cop0.gpr[12] |= (1 << 1);

        ctx.pc = target - 4;
    }

    branch_delay = false;
    branch = false;
}

void Interpreter::RaiseInterrupt(int signal, bool value) {
    if (signal == 0) {
        ctx.cop0.cause.int0_pending = value;
    } else {
        // int1 signal
        ctx.cop0.cause.int1_pending = value;
    }
}

void Interpreter::CheckInterrupts() {
    if (InterruptsEnabled()) {
        bool int0_enable = (ctx.cop0.gpr[12] >> 10) & 0x1;
        bool timer_enable = (ctx.cop0.gpr[12] >> 15) & 0x1;

        assert(timer_enable == false);
        
        if (int0_enable && ctx.cop0.cause.int0_pending) {
            common::Log("[ee::Interpreter] do int0 interrupt");
            DoException(0x80000200, ExceptionType::Interrupt);
            return;
        }

        bool int1_enable = (ctx.cop0.gpr[12] >> 11) & 0x1;
        
        if (int1_enable && ctx.cop0.cause.int1_pending) {
            common::Log("[ee::Interpreter] do int1 interrupt");
            DoException(0x80000200, ExceptionType::Interrupt);
            return;
        }

        // TODO: handle cop0 compare interrupts
    }
}

bool Interpreter::InterruptsEnabled() {
    bool ie = ctx.cop0.gpr[12] & 0x1;
    bool eie = (ctx.cop0.gpr[12] >> 16) & 0x1;
    bool exl = (ctx.cop0.gpr[12] >> 1) & 0x1;
    bool erl = (ctx.cop0.gpr[12] >> 2) & 0x1;

    return ie && eie && !exl && !erl;
}

void Interpreter::LogState() {
    common::Log("[EE State]");
    for (int i = 0; i < 32; i++) {
        common::Log("%s: %016lx%016lx", GetRegisterName(i).c_str(), ctx.GetReg<u128>(i).hi, ctx.GetReg<u128>(i).lo);
    }

    common::Log("pc: %08x npc: %08x", ctx.pc, ctx.npc);
    common::Log("branch: %d branch delay: %d", branch, branch_delay);
    common::Log("%s", DisassembleInstruction(inst, ctx.pc).c_str());
}

void Interpreter::LogInstruction() {
    common::Log("[ee::Interpreter] %08x %08x %s", ctx.pc, inst.data, DisassembleInstruction(inst, ctx.pc).c_str());
}

void Interpreter::Branch(bool cond) {
    s32 offset = inst.simm << 2;

    if (cond) {
        ctx.npc = ctx.pc + offset + 4;
        branch_delay = true;
    }
}

void Interpreter::BranchLikely(bool cond) {
    s32 offset = inst.simm << 2;

    if (cond) {
        ctx.npc = ctx.pc + offset + 4;
        branch_delay = true;
    } else {
        ctx.pc += 4;
    }
}

void Interpreter::Jump(u32 target) {
    ctx.npc = target;
    branch_delay = true;
}

// COP0 instructions
void Interpreter::mfc0() {
    ctx.SetReg<s64>(inst.rt, static_cast<s32>(ctx.cop0.GetReg(inst.rd)));
}

void Interpreter::mtc0() {
    ctx.cop0.SetReg(inst.rd, ctx.GetReg<u32>(inst.rt));
}

// COP1 instructions
// TODO: do instruction execution in the cop1
void Interpreter::swc1() {
    u32 vaddr = ctx.GetReg<u32>(inst.rs) + inst.simm;

    if (vaddr & 0x3) {
        common::Error("[ee::Interpreter] handle unaligned swc1 vaddr %08x", vaddr);
    }
    
    ctx.Write<u32>(vaddr, ctx.cop1.GetReg(inst.rt));
}

void Interpreter::mtc1() {
    ctx.cop1.SetReg(inst.rd, ctx.GetReg<u32>(inst.rt));
}

void Interpreter::adda_s() {
    ctx.cop1.accumulator.f = ctx.cop1.AsFloat(ctx.cop1.GetReg(inst.rd)) + ctx.cop1.AsFloat(ctx.cop1.GetReg(inst.rt));

    // TODO: handle overflows and underflows
}

void Interpreter::ctc1() {
    ctx.cop1.SetControlReg(inst.rd, ctx.GetReg<u32>(inst.rt));
}

void Interpreter::cfc1() {
    switch (inst.rd) {
    case 0:
    case 31:
        ctx.SetReg<u64>(inst.rt, static_cast<s32>(ctx.cop1.GetControlReg(inst.rd)));
        break;
    default:
        common::Error("[ee::Interpreter] operation where fs != 0 or fs != 31 is undefined");
    }
}

void Interpreter::madd_s() {
    float fs = ctx.cop1.AsFloat(ctx.cop1.GetReg(inst.rd));
    float ft = ctx.cop1.AsFloat(ctx.cop1.GetReg(inst.rt));
    float acc = ctx.cop1.AsFloat(ctx.cop1.accumulator.u);

    ctx.cop1.SetReg(inst.imm5, acc + (fs * ft));

    // TODO: handle overflows and underflows
}

void Interpreter::lwc1() {
    u32 vaddr = ctx.GetReg<u32>(inst.rs) + inst.simm;

    if (vaddr & 0x3) {
        common::Error("[ee::Interpreter] handle unaligned lwc1 vaddr", vaddr);
    }

    ctx.cop1.SetReg(inst.rt, ctx.Read<u32>(vaddr));
}

// COP2 instructions
void Interpreter::cfc2() {
    // handle vu stuff
}

void Interpreter::ctc2() {
    // handle vu stuff
}

// MMI instructions
void Interpreter::divu1() {
    if (ctx.GetReg<u32>(inst.rt)) {
        ctx.lo1 = SignExtend<s64, 32>(ctx.GetReg<u32>(inst.rs) / ctx.GetReg<u32>(inst.rt));
        ctx.hi1 = SignExtend<s64, 32>(ctx.GetReg<u32>(inst.rs) % ctx.GetReg<u32>(inst.rt));
    } else {
        ctx.lo1 = 0xFFFFFFFFFFFFFFFF;
        ctx.hi1 = SignExtend<s64, 32>(ctx.GetReg<u32>(inst.rs));
    }
}

void Interpreter::mflo1() {
    ctx.SetReg<u64>(inst.rd, ctx.lo1);
}

void Interpreter::mult1() {
    s64 result = ctx.GetReg<s32>(inst.rt) * ctx.GetReg<s32>(inst.rs);
    ctx.lo1 = SignExtend<s64, 32>(result & 0xFFFFFFFF);
    ctx.hi1 = SignExtend<s64, 32>(result >> 32);
    ctx.SetReg<u64>(inst.rd, ctx.lo1);
}

void Interpreter::por() {
    ctx.SetReg<u128>(inst.rd, ctx.GetReg<u128>(inst.rs) | ctx.GetReg<u128>(inst.rt));
}

void Interpreter::padduw() {
    for (int i = 0; i < 4; i++) {
        u64 result = ctx.GetReg<u32>(inst.rs, i) + ctx.GetReg<u32>(inst.rt, i);

        if (result > 0xFFFFFFFF) {
            ctx.SetReg<u32>(inst.rd, 0xFFFFFFFF, i);
        } else {
            ctx.SetReg<u32>(inst.rd, result);
        }
    }
}

void Interpreter::mfhi1() {
    ctx.SetReg<u64>(inst.rd, ctx.hi1);
}

void Interpreter::plzcw() {
    for (int i = 0; i < 2; i++) {
        u32 data = ctx.GetReg<u32>(inst.rs, i);
        ctx.SetReg<u32>(inst.rd, CountLeadingSignBits(data) - 1, i);
    }
}

void Interpreter::mthi1() {
    ctx.hi1 = ctx.GetReg<u64>(inst.rs);
}

void Interpreter::mtlo1() {
    ctx.lo1 = ctx.GetReg<u64>(inst.rs);
}

// primary instructions
void Interpreter::slti() {
    ctx.SetReg<u64>(inst.rt, ctx.GetReg<s64>(inst.rs) < SignExtend<s64, 16>(inst.imm));
}

void Interpreter::bne() {
    Branch(ctx.GetReg<u64>(inst.rs) != ctx.GetReg<u64>(inst.rt));
}

void Interpreter::lui() {
    ctx.SetReg<u64>(inst.rt, SignExtend<s64, 32>(inst.imm << 16));
}

void Interpreter::ori() {
    ctx.SetReg<u64>(inst.rt, ctx.GetReg<u64>(inst.rs) | inst.imm);
}

void Interpreter::addiu() {
    s32 result = ctx.GetReg<s64>(inst.rs) + (s16)inst.imm;
    ctx.SetReg<s64>(inst.rt, result);
}

void Interpreter::sw() {
    ctx.Write<u32>(ctx.GetReg<u32>(inst.rs) + inst.simm, ctx.GetReg<u32>(inst.rt));
}

void Interpreter::sd() {
    ctx.Write<u64>(ctx.GetReg<u32>(inst.rs) + inst.simm, ctx.GetReg<u64>(inst.rt));
}

void Interpreter::jal() {
    ctx.SetReg<u64>(31, ctx.pc + 8);
    Jump(((ctx.pc + 4) & 0xf0000000) + (inst.offset << 2));
}

void Interpreter::andi() {
    ctx.SetReg<u64>(inst.rt, ctx.GetReg<u64>(inst.rs) & inst.imm);
}

void Interpreter::beq() {
    Branch(ctx.GetReg<u64>(inst.rs) == ctx.GetReg<u64>(inst.rt));
}

void Interpreter::beql() {
    BranchLikely(ctx.GetReg<u64>(inst.rs) == ctx.GetReg<u64>(inst.rt));
}

void Interpreter::sltiu() {
    ctx.SetReg<u64>(inst.rt, ctx.GetReg<u64>(inst.rs) < (u64)(s64)inst.simm);
}

void Interpreter::bnel() {
    BranchLikely(ctx.GetReg<u64>(inst.rs) != ctx.GetReg<u64>(inst.rt));
}

void Interpreter::lb() {
    ctx.SetReg<s64>(inst.rt, (s8)ctx.Read<u8>(ctx.GetReg<u32>(inst.rs) + inst.simm));
}

void Interpreter::lbu() {
    ctx.SetReg<u64>(inst.rt, ctx.Read<u8>(ctx.GetReg<u32>(inst.rs) + inst.simm));
}

void Interpreter::ld() {
    ctx.SetReg<u64>(inst.rt, ctx.Read<u64>(ctx.GetReg<u32>(inst.rs) + inst.simm));
}

void Interpreter::j() {
    Jump(((ctx.pc + 4) & 0xF0000000) + (inst.offset << 2));
}

void Interpreter::lw() {
    ctx.SetReg<s64>(inst.rt, (s32)ctx.Read<u32>(ctx.GetReg<u32>(inst.rs) + inst.simm));
}

void Interpreter::sb() {
    ctx.Write<u8>(ctx.GetReg<u32>(inst.rs) + inst.simm, ctx.GetReg<u8>(inst.rt));
}

void Interpreter::blez() {
    Branch(ctx.GetReg<s64>(inst.rs) <= 0);
}

void Interpreter::lhu() {
    ctx.SetReg<u64>(inst.rt, ctx.Read<u16>(ctx.GetReg<u32>(inst.rs) + inst.simm));
}

void Interpreter::bgtz() {
    Branch(ctx.GetReg<s64>(inst.rs) > 0);
}

void Interpreter::sh() {
    ctx.Write<u16>(ctx.GetReg<u32>(inst.rs) + inst.simm, ctx.GetReg<u16>(inst.rt));
}

void Interpreter::xori() {
    ctx.SetReg<u64>(inst.rt, ctx.GetReg<u64>(inst.rs) ^ inst.imm);
}

void Interpreter::daddiu() {
    ctx.SetReg<s64>(inst.rt, ctx.GetReg<s64>(inst.rs) + SignExtend<s64, 16>(inst.imm));
}

void Interpreter::sq() {
    u128 reg = ctx.GetReg<u128>(inst.rt);
    u32 addr = (ctx.GetReg<u32>(inst.rs) + inst.simm) & ~0xF;

    ctx.Write<u128>(addr, reg);
}

void Interpreter::lq() {
    u32 vaddr = (ctx.GetReg<u32>(inst.rs) + inst.simm) & ~0xF;
    u128 data = ctx.Read<u128>(vaddr);
    ctx.SetReg<u128>(inst.rt, data);
}

void Interpreter::lh() {
    ctx.SetReg<s64>(inst.rt, (s16)ctx.Read<u16>(ctx.GetReg<u32>(inst.rs) + inst.simm));
}

void Interpreter::cache() {
    // handle later
}

void Interpreter::lwu() {
    ctx.SetReg<u64>(inst.rt, ctx.Read<u32>(ctx.GetReg<u32>(inst.rs) + inst.simm));
}

void Interpreter::ldl() {
    const u64 mask[8] = {
        0x00FFFFFFFFFFFFFF, 0x0000FFFFFFFFFFFF, 0x000000FFFFFFFFFF, 0x00000000FFFFFFFF,
        0x0000000000FFFFFF, 0x000000000000FFFF, 0x00000000000000FF, 0x0000000000000000,
    };

    const u8 shift[8] = {56, 48, 40, 32, 24, 16, 8, 0};

    u32 addr = ctx.GetReg<u32>(inst.rs) + inst.simm;
    int index = addr & 0x7;
    u64 data = ctx.Read<u64>(addr & ~0x7);
    u64 reg = ctx.GetReg<u64>(inst.rt);
    ctx.SetReg<u64>(inst.rt, (reg & mask[index]) | (data << shift[index]));
}

void Interpreter::ldr() {
    const u64 mask[8] = {
        0x0000000000000000, 0xFF00000000000000, 0xFFFF000000000000, 0xFFFFFF0000000000,
        0xFFFFFFFF00000000, 0xFFFFFFFFFF000000, 0xFFFFFFFFFFFF0000, 0xFFFFFFFFFFFFFF00,
    };

    const u8 shift[8] = {0, 8, 16, 24, 32, 40, 48, 56};

    u32 addr = ctx.GetReg<u32>(inst.rs) + inst.simm;
    int index = addr & 0x7;
    u64 data = ctx.Read<u64>(addr & ~0x7);
    u64 reg = ctx.GetReg<u64>(inst.rt);
    ctx.SetReg<u64>(inst.rt, (reg & mask[index]) | (data >> shift[index]));
}

void Interpreter::sdl() {
    const u64 mask[8] = {
        0xFFFFFFFFFFFFFF00, 0xFFFFFFFFFFFF0000, 0xFFFFFFFFFF000000, 0xFFFFFFFF00000000,
        0xFFFFFF0000000000, 0xFFFF000000000000, 0xFF00000000000000, 0x0000000000000000,
    };

    const u8 shift[8] = {56, 48, 40, 32, 24, 16, 8, 0};

    u32 addr = ctx.GetReg<u32>(inst.rs) + inst.simm;
    int index = addr & 0x7;

    u64 data = ctx.Read<u64>(addr & ~0x7);
    u64 reg = ctx.GetReg<u64>(inst.rt);
    ctx.Write<u64>(addr & ~0x7, (reg >> shift[index]) | (data & mask[index]));
}

void Interpreter::sdr() {
    const u64 mask[8] = {
        0x0000000000000000, 0x00000000000000ff, 0x000000000000ffff, 0x0000000000ffffff,
        0x00000000ffffffff, 0x000000ffffffffff, 0x0000ffffffffffff, 0x00ffffffffffffff,
    };

    const u8 shift[8] = {0, 8, 16, 24, 32, 40, 48, 56};

    u32 addr = ctx.GetReg<u32>(inst.rs) + inst.simm;
    int index = addr & 0x7;

    u64 data = ctx.Read<u64>(addr & ~0x7);
    u64 reg = ctx.GetReg<u64>(inst.rt);
    ctx.Write<u64>(addr & ~0x7, (reg << shift[index]) | (data & mask[index]));
}

// regimm instructions
void Interpreter::bgez() {
    Branch(ctx.GetReg<s64>(inst.rs) >= 0);
}

void Interpreter::bltz() {
    Branch(ctx.GetReg<s64>(inst.rs) < 0);
}

void Interpreter::bltzl() {
    BranchLikely(ctx.GetReg<s64>(inst.rs) < 0);
}

void Interpreter::bgezl() {
    BranchLikely(ctx.GetReg<s64>(inst.rs) >= 0);
}

void Interpreter::bgezal() {
    ctx.SetReg<u64>(31, ctx.pc + 8);
    BranchLikely(ctx.GetReg<s64>(inst.rs) >= 0);
}

// secondary instructions
void Interpreter::sll() {
    u32 result = ctx.GetReg<u32>(inst.rt) << inst.imm5;
    ctx.SetReg<s64>(inst.rd, (s32)result);
}

void Interpreter::jr() {
    Jump(ctx.GetReg<u32>(inst.rs));
}

void Interpreter::sync() {
    
}

void Interpreter::jalr() {
    ctx.SetReg<u64>(inst.rd, ctx.pc + 8);
    Jump(ctx.GetReg<u32>(inst.rs));
}

void Interpreter::daddu() {
    ctx.SetReg<u64>(inst.rd, ctx.GetReg<s64>(inst.rs) + ctx.GetReg<s64>(inst.rt));
}

void Interpreter::orr() {
    ctx.SetReg<u64>(inst.rd, ctx.GetReg<u64>(inst.rs) | ctx.GetReg<u64>(inst.rt));
}

void Interpreter::mult() {
    s64 result = ctx.GetReg<s32>(inst.rs) * ctx.GetReg<s32>(inst.rt);
    ctx.lo = (s32)(result & 0xFFFFFFFF);
    ctx.hi = (s32)(result >> 32);
    ctx.SetReg<u64>(inst.rd, ctx.lo);
}

void Interpreter::divu() {
    if (ctx.GetReg<u32>(inst.rt) == 0) {
        ctx.lo = 0xFFFFFFFFFFFFFFFF;
        ctx.hi = SignExtend<s64, 32>(ctx.GetReg<u32>(inst.rs));
    } else {
        ctx.lo = SignExtend<s64, 32>(ctx.GetReg<u32>(inst.rs) / ctx.GetReg<u32>(inst.rt));
        ctx.hi = SignExtend<s64, 32>(ctx.GetReg<u32>(inst.rs) % ctx.GetReg<u32>(inst.rt));
    }
}

void Interpreter::break_exception() {
    // not sure what to do here
}

void Interpreter::mflo() {
    ctx.SetReg<u64>(inst.rd, ctx.lo);
}

void Interpreter::srl() {
    u32 result = ctx.GetReg<u32>(inst.rt) >> inst.imm5;
    ctx.SetReg<s64>(inst.rd, (s32)result);
}

void Interpreter::sra() {
    s32 result = ctx.GetReg<s32>(inst.rt) >> inst.imm5;
    ctx.SetReg<s64>(inst.rd, result);
}

void Interpreter::slt() {
    ctx.SetReg<u64>(inst.rd, ctx.GetReg<s64>(inst.rs) < ctx.GetReg<s64>(inst.rt));
}

void Interpreter::addu() {
    s32 result = ctx.GetReg<s64>(inst.rs) + ctx.GetReg<s64>(inst.rt);
    ctx.SetReg<s64>(inst.rd, result);
}

void Interpreter::sltu() {
    ctx.SetReg<u64>(inst.rd, ctx.GetReg<u64>(inst.rs) < ctx.GetReg<u64>(inst.rt));
}

void Interpreter::andd() {
    ctx.SetReg<u64>(inst.rd, ctx.GetReg<u64>(inst.rs) & ctx.GetReg<u64>(inst.rt));
}

void Interpreter::movn() {
    if (ctx.GetReg<u64>(inst.rt)) {
        ctx.SetReg<u64>(inst.rd, ctx.GetReg<u64>(inst.rs));
    }
}

void Interpreter::subu() {
    s32 result = ctx.GetReg<s32>(inst.rs) - ctx.GetReg<s32>(inst.rt);
    ctx.SetReg<s64>(inst.rd, result);
}

void Interpreter::div() {
    if ((ctx.GetReg<s32>(inst.rs) == (s32)0x80000000) && (ctx.GetReg<s32>(inst.rt) == -1)) {
        ctx.lo = 0x80000000;
        ctx.hi = 0;
    } else if (ctx.GetReg<s32>(inst.rt) == 0) {
        common::Error("can't divide by 0");
    } else {
        ctx.lo = SignExtend<s64, 32>(ctx.GetReg<s32>(inst.rs) / ctx.GetReg<s32>(inst.rt));
        ctx.hi = SignExtend<s64, 32>(ctx.GetReg<s32>(inst.rs) % ctx.GetReg<s32>(inst.rt));
    }
}

void Interpreter::mfhi() {
    ctx.SetReg<u64>(inst.rd, ctx.hi);
}

void Interpreter::dsrav() {
    ctx.SetReg<s64>(inst.rd, ctx.GetReg<s64>(inst.rt) >> (ctx.GetReg<u8>(inst.rs) & 0x3F));
}

void Interpreter::dsll32() {
    ctx.SetReg<u64>(inst.rd, ctx.GetReg<u64>(inst.rt) << (32 + inst.imm5));
}

void Interpreter::dsra32() {
    ctx.SetReg<s64>(inst.rd, ctx.GetReg<s64>(inst.rt) >> (32 + inst.imm5));
}

void Interpreter::movz() {
    if (ctx.GetReg<u64>(inst.rt) == 0) {
        ctx.SetReg<u64>(inst.rd, ctx.GetReg<u64>(inst.rs));
    }
}

void Interpreter::dsllv() {
    ctx.SetReg<u64>(inst.rd, ctx.GetReg<u64>(inst.rt) << (ctx.GetReg<u8>(inst.rs) & 0x3F));
}

void Interpreter::sllv() {
    u32 result = ctx.GetReg<u32>(inst.rt) << (ctx.GetReg<u8>(inst.rs) & 0x1F);
    ctx.SetReg<s64>(inst.rd, SignExtend<s64, 32>(result));
}

void Interpreter::dsll() {
    ctx.SetReg<u64>(inst.rd, ctx.GetReg<u64>(inst.rt) << inst.imm5);
}

void Interpreter::srav() {
    s32 result = ctx.GetReg<s32>(inst.rt) >> (ctx.GetReg<u8>(inst.rs) & 0x1F);
    ctx.SetReg<s64>(inst.rd, result);
}

void Interpreter::nor() {
    ctx.SetReg<u64>(inst.rd, ~(ctx.GetReg<u64>(inst.rs) | ctx.GetReg<u64>(inst.rt)));
}

void Interpreter::dsrl() {
    ctx.SetReg<u64>(inst.rd, ctx.GetReg<u64>(inst.rt) >> inst.imm5);
}

void Interpreter::srlv() {
    s32 result = ctx.GetReg<u32>(inst.rt) >> (ctx.GetReg<u8>(inst.rs) & 0x1F);
    ctx.SetReg<s64>(inst.rd, result);
}

void Interpreter::dsrl32() {
    ctx.SetReg<u64>(inst.rd, ctx.GetReg<u64>(inst.rt) >> (32 + inst.imm5));
}

void Interpreter::syscall_exception() {
    u8 opcode = ctx.Read<u8>(ctx.pc - 4);

    common::Log("[ee::Interpreter] executing syscall %s", ctx.GetSyscallInfo(opcode).c_str());
    DoException(0x80000180, ExceptionType::Syscall);
}

void Interpreter::dsubu() {
    ctx.SetReg<u64>(inst.rd, ctx.GetReg<s64>(inst.rs) - ctx.GetReg<s64>(inst.rt));
}

void Interpreter::mfsa() {
    ctx.SetReg<u64>(inst.rd, ctx.sa);
}

void Interpreter::mthi() {
    ctx.hi = ctx.GetReg<u64>(inst.rs);
}

void Interpreter::mtlo() {
    ctx.lo = ctx.GetReg<u64>(inst.rs);
}

void Interpreter::mtsa() {
    ctx.sa = ctx.GetReg<u64>(inst.rs);
}

void Interpreter::dsra() {
    ctx.SetReg<s64>(inst.rd, ctx.GetReg<s64>(inst.rt) >> inst.imm5);
}

void Interpreter::dsrlv() {
    ctx.SetReg<u64>(inst.rd, ctx.GetReg<u64>(inst.rt) >> (ctx.GetReg<u8>(inst.rs) & 0x3f));
}

void Interpreter::xorr() {
    ctx.SetReg<u64>(inst.rd, ctx.GetReg<u64>(inst.rs) ^ ctx.GetReg<u64>(inst.rt));
}

// tlb instructions
void Interpreter::tlbwi() {
    common::Log("[ee::Interpreter] tlbwi entry %d", ctx.cop0.index);
    // when we handle mmu emulation the tlb will be used
}

void Interpreter::di() {
    u32 status = ctx.cop0.GetReg(12);
    bool edi = status & (1 << 17);
    bool exl = status & (1 << 1);
    bool erl = status & (1 << 2);
    bool ksu = ((status >> 3) & 0x3) == 0;

    if (edi || exl || erl || ksu) {
        ctx.cop0.SetReg(12, status & ~(1 << 16));
    } 
}

void Interpreter::eret() {
    u32 status = ctx.cop0.GetReg(12);
    bool erl = status & (1 << 2);
    u32 errorepc = ctx.cop0.GetReg(30);
    u32 epc = ctx.cop0.GetReg(14);

    if (erl) {
        ctx.pc = errorepc;
        ctx.cop0.SetReg(12, status & ~(1 << 2));
    } else {
        ctx.pc = epc;
        ctx.cop0.SetReg(12, status & ~(1 << 1));
    }

    if (ctx.system.boot_mode == BootMode::Fast && !ctx.system.fastboot_done && ctx.pc == 0x82000) {
        ctx.system.fastboot_done = true;
        ctx.system.elf_loader.Load();
    }

    // this is to account for the increment by 4 that
    // happens after each instruction is executed
    ctx.pc -= 4;
}

void Interpreter::ei() {
    u32 status = ctx.cop0.GetReg(12);
    bool edi = status & (1 << 17);
    bool exl = status & (1 << 1);
    bool erl = status & (1 << 2);
    bool ksu = ((status >> 3) & 0x3) == 0;

    if (edi || exl || erl || ksu) {
        ctx.cop0.SetReg(12, status | (1 << 16));
    }
}

void Interpreter::illegal_instruction() {
    common::Error("%s = %08x at %08x (primary = %d, secondary = %d, regimm = %d, rs = %d, imm5 = %d) is undefined", DisassembleInstruction(inst, ctx.pc).c_str(), inst.data, ctx.pc, inst.opcode, inst.func, inst.rt, inst.rs, inst.imm5);
}

void Interpreter::stub_instruction() {
    common::Log("%s = %08x at %08x (primary = %d, secondary = %d, regimm = %d) is undefined", DisassembleInstruction(inst, ctx.pc).c_str(), inst.data, ctx.pc, inst.opcode, inst.func, inst.rt);
}

} // namespace ee