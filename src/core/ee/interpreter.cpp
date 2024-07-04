#include <cassert>
#include "common/types.h"
#include "common/log.h"
#include "common/bits.h"
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
        inst = Instruction{ctx.read<u32>(ctx.pc)};
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
void Interpreter::swc1() {
    u32 vaddr = ctx.GetReg<u32>(inst.rs) + inst.simm;

    if (vaddr & 0x3) {
        common::Error("[ee::Interpreter] handle unaligned swc1 vaddr %08x", vaddr);
    }
    
    ctx.write<u32>(vaddr, ctx.cop1.GetReg(inst.ft));
}

void Interpreter::mtc1() {
    ctx.cop1.SetReg(inst.fs, ctx.GetReg<u32>(inst.rt));
}

void Interpreter::adda_s() {
    ctx.cop1.adda_s(inst);
}

void Interpreter::ctc1() {
    ctx.cop1.SetControlReg(inst.fs, ctx.GetReg<u32>(inst.rt));
}

void Interpreter::cfc1() {
    ctx.SetReg<u64>(inst.rt, static_cast<s32>(ctx.cop1.GetControlReg(inst.fs)));
}

void Interpreter::madd_s() {
    ctx.cop1.madd_s(inst);
}

void Interpreter::lwc1() {
    u32 vaddr = ctx.GetReg<u32>(inst.rs) + inst.simm;

    if (vaddr & 0x3) {
        common::Error("[ee::Interpreter] handle unaligned lwc1 vaddr", vaddr);
    }

    ctx.cop1.SetReg(inst.ft, ctx.read<u32>(vaddr));
}

void Interpreter::mov_s() {
    ctx.cop1.mov_s(inst);
}

void Interpreter::abs_s() {
    ctx.cop1.abs_s(inst);
}

void Interpreter::add_s() {
    ctx.cop1.add_s(inst);
}

void Interpreter::max_s() {
    ctx.cop1.max_s(inst);
}

void Interpreter::min_s() {
    ctx.cop1.min_s(inst);
}

void Interpreter::neg_s() {
    ctx.cop1.neg_s(inst);
}

void Interpreter::sub_s() {
    ctx.cop1.sub_s(inst);
}

void Interpreter::suba_s() {
    ctx.cop1.suba_s(inst);
}

void Interpreter::c_eq_s() {
    ctx.cop1.c_eq_s(inst);
}

void Interpreter::c_f_s() {
    ctx.cop1.c_f_s();
}

// bc1 instructions
void Interpreter::bc1f() {
    Branch(!ctx.cop1.condition());
}

void Interpreter::bc1fl() {
    BranchLikely(!ctx.cop1.condition());
}

void Interpreter::bc1t() {
    Branch(ctx.cop1.condition());
}

void Interpreter::bc1tl() {
    BranchLikely(ctx.cop1.condition());
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
        ctx.lo1 = common::SignExtend<s64, 32>(ctx.GetReg<u32>(inst.rs) / ctx.GetReg<u32>(inst.rt));
        ctx.hi1 = common::SignExtend<s64, 32>(ctx.GetReg<u32>(inst.rs) % ctx.GetReg<u32>(inst.rt));
    } else {
        ctx.lo1 = 0xFFFFFFFFFFFFFFFF;
        ctx.hi1 = common::SignExtend<s64, 32>(ctx.GetReg<u32>(inst.rs));
    }
}

void Interpreter::mflo1() {
    ctx.SetReg<u64>(inst.rd, ctx.lo1);
}

void Interpreter::mult1() {
    s64 result = static_cast<s64>(ctx.GetReg<s32>(inst.rs)) * static_cast<s64>(ctx.GetReg<s32>(inst.rt));
    ctx.lo1 = common::SignExtend<s64, 32>(result & 0xffffffff);
    ctx.hi1 = common::SignExtend<s64, 32>(result >> 32);
    ctx.SetReg<u64>(inst.rd, ctx.lo1);
}

void Interpreter::por() {
    ctx.SetReg<u128>(inst.rd, ctx.GetReg<u128>(inst.rs) | ctx.GetReg<u128>(inst.rt));
}

void Interpreter::mfhi1() {
    ctx.SetReg<u64>(inst.rd, ctx.hi1);
}

void Interpreter::plzcw() {
    for (int i = 0; i < 2; i++) {
        u32 data = ctx.GetReg<u32>(inst.rs, i);
        ctx.SetReg<u32>(inst.rd, common::CountLeadingSignBits(data) - 1, i);
    }
}

void Interpreter::mthi1() {
    ctx.hi1 = ctx.GetReg<u64>(inst.rs);
}

void Interpreter::mtlo1() {
    ctx.lo1 = ctx.GetReg<u64>(inst.rs);
}

void Interpreter::pcpyld() {
    u64 lower = ctx.GetReg<u64>(inst.rt);
    u64 upper = ctx.GetReg<u64>(inst.rs);
    ctx.SetReg<u64>(inst.rd, lower);
    ctx.SetReg<u64>(inst.rd, upper, 1);
}

void Interpreter::pnor() {
    ctx.SetReg<u128>(inst.rd, ~(ctx.GetReg<u128>(inst.rs) | ctx.GetReg<u128>(inst.rt)));
}

void Interpreter::pand() {
    ctx.SetReg<u128>(inst.rd, ctx.GetReg<u128>(inst.rs) & ctx.GetReg<u128>(inst.rt));
}

void Interpreter::pxor() {
    ctx.SetReg<u128>(inst.rd, ctx.GetReg<u128>(inst.rs) ^ ctx.GetReg<u128>(inst.rt));
}

void Interpreter::pcpyud() {
    ctx.SetReg<u64>(inst.rd, ctx.GetReg<u64>(inst.rs, 1));
    ctx.SetReg<u64>(inst.rd, ctx.GetReg<u64>(inst.rt, 1), 1);
}

void Interpreter::pcpyh() {
    u16 lower = ctx.GetReg<u16>(inst.rt);
    u16 upper = ctx.GetReg<u16>(inst.rt, 4);
    for (int i = 0; i < 4; i++) {
        ctx.SetReg<u16>(inst.rd, lower, i);
        ctx.SetReg<u16>(inst.rd, upper, 4 + i);
    }
}

void Interpreter::div1() {
    if ((ctx.GetReg<s32>(inst.rs) == static_cast<s32>(0x80000000)) && (ctx.GetReg<s32>(inst.rt) == -1)) {
        ctx.lo1 = common::SignExtend<s64, 32>(0x80000000);
        ctx.hi1 = 0;
    } else if (ctx.GetReg<s32>(inst.rt) == 0) {
        if (ctx.GetReg<s32>(inst.rs) >= 0) {
            ctx.lo1 = common::SignExtend<s64, 32>(0xffffffff);
        } else {
            ctx.lo1 = 1;
        }
        ctx.hi1 = common::SignExtend<s64, 32>(ctx.GetReg<s32>(inst.rs));
    } else {
        ctx.lo1 = common::SignExtend<s64, 32>(ctx.GetReg<s32>(inst.rs) / ctx.GetReg<s32>(inst.rt));
        ctx.hi1 = common::SignExtend<s64, 32>(ctx.GetReg<s32>(inst.rs) % ctx.GetReg<s32>(inst.rt));
    }
}

void Interpreter::madd() {
    u64 lo = ctx.lo & 0xffffffff;
    u64 hi = ctx.hi & 0xffffffff;
    s64 result = ((hi << 32) | lo) + (static_cast<s64>(ctx.GetReg<s32>(inst.rs)) * static_cast<s64>(ctx.GetReg<s32>(inst.rt)));
    ctx.lo = common::SignExtend<s64, 32>(result & 0xffffffff);
    ctx.hi = common::SignExtend<s64, 32>(result >> 32);
    ctx.SetReg<s64>(inst.rd, static_cast<s64>(ctx.lo));
}

void Interpreter::maddu() {
    u64 lo = ctx.lo & 0xffffffff;
    u64 hi = ctx.hi & 0xffffffff;
    u64 result = ((hi << 32) | lo) + (static_cast<u64>(ctx.GetReg<u32>(inst.rs)) * static_cast<u64>(ctx.GetReg<u32>(inst.rt)));
    ctx.lo = common::SignExtend<s64, 32>(result & 0xffffffff);
    ctx.hi = common::SignExtend<s64, 32>(result >> 32);
    ctx.SetReg<s64>(inst.rd, static_cast<s64>(ctx.lo));
}

void Interpreter::madd1() {
    u64 lo1 = ctx.lo1 & 0xffffffff;
    u64 hi1 = ctx.hi1 & 0xffffffff;
    s64 result = ((hi1 << 32) | lo1) + (static_cast<s64>(ctx.GetReg<s32>(inst.rs)) * static_cast<s64>(ctx.GetReg<s32>(inst.rt)));
    ctx.lo1 = common::SignExtend<s64, 32>(result & 0xffffffff);
    ctx.hi1 = common::SignExtend<s64, 32>(result >> 32);
    ctx.SetReg<s64>(inst.rd, static_cast<s64>(ctx.lo1));
}

void Interpreter::maddu1() {
    u64 lo1 = ctx.lo1 & 0xffffffff;
    u64 hi1 = ctx.hi1 & 0xffffffff;
    u64 result = ((hi1 << 32) | lo1) + (static_cast<u64>(ctx.GetReg<u32>(inst.rs)) * static_cast<u64>(ctx.GetReg<u32>(inst.rt)));
    ctx.lo1 = common::SignExtend<s64, 32>(result & 0xffffffff);
    ctx.hi1 = common::SignExtend<s64, 32>(result >> 32);
    ctx.SetReg<s64>(inst.rd, static_cast<s64>(ctx.lo1));
}

void Interpreter::multu1() {
    u64 result = static_cast<u64>(ctx.GetReg<u32>(inst.rs)) * static_cast<u64>(ctx.GetReg<u32>(inst.rt));
    ctx.lo1 = common::SignExtend<s64, 32>(result & 0xffffffff);
    ctx.hi1 = common::SignExtend<s64, 32>(result >> 32);
    ctx.SetReg<u64>(inst.rd, ctx.lo1);
}

void Interpreter::pabsh() {
    for (int i = 0; i < 8; i++) {
        s16 signed_data = ctx.GetReg<s16>(inst.rt, i);
        u16 data = ctx.GetReg<u16>(inst.rt, i);
        if (data == 0x8000) {
            ctx.SetReg<u16>(inst.rd, 0x7fff, i);
        } else if (signed_data < 0) {
            ctx.SetReg<u16>(inst.rd, -signed_data, i);
        } else {
            ctx.SetReg<u16>(inst.rd, data, i);
        }
    }
}

void Interpreter::pabsw() {
    for (int i = 0; i < 4; i++) {
        s32 signed_data = ctx.GetReg<s32>(inst.rt, i);
        u32 data = ctx.GetReg<u32>(inst.rt, i);
        if (data == 0x80000000) {
            ctx.SetReg<u32>(inst.rd, 0x7fffffff, i);
        } else if (signed_data < 0) {
            ctx.SetReg<u32>(inst.rd, -signed_data, i);
        } else {
            ctx.SetReg<u32>(inst.rd, data, i);
        }
    }
}

void Interpreter::paddb() {
    for (int i = 0; i < 16; i++) {
        ctx.SetReg<s8>(inst.rd, ctx.GetReg<s8>(inst.rs, i) + ctx.GetReg<s8>(inst.rt, i), i);
    }
}

void Interpreter::paddh() {
    for (int i = 0; i < 8; i++) {
        ctx.SetReg<s16>(inst.rd, ctx.GetReg<s16>(inst.rs, i) + ctx.GetReg<s16>(inst.rt, i), i);
    }
}

void Interpreter::paddsb() {
    for (int i = 0; i < 16; i++) {
        s16 result = static_cast<s16>(ctx.GetReg<s8>(inst.rs, i)) + static_cast<s16>(ctx.GetReg<s8>(inst.rt, i));
        if (result > 0x7f) {
            result = 0x7f;
        } else if (result < -0x80) {
            result = -0x80;
        }
        ctx.SetReg<s8>(inst.rd, result & 0xff, i);
    }
}

void Interpreter::paddsh() {
    for (int i = 0; i < 8; i++) {
        s32 result = static_cast<s32>(ctx.GetReg<s16>(inst.rs, i)) + static_cast<s32>(ctx.GetReg<s16>(inst.rt, i));
        if (result > 0x7fff) {
            result = 0x7fff;
        } else if (result < -0x8000) {
            result = -0x8000;
        }
        ctx.SetReg<s16>(inst.rd, result & 0xffff, i);
    }
}

void Interpreter::paddsw() {
    for (int i = 0; i < 4; i++) {
        s64 result = static_cast<s64>(ctx.GetReg<s32>(inst.rs, i)) + static_cast<s64>(ctx.GetReg<s32>(inst.rt, i));
        if (result > 0x7fffffff) {
            result = 0x7fffffff;
        } else if (result < static_cast<s32>(0x80000000)) {
            result = static_cast<s32>(0x80000000);
        }
        ctx.SetReg<s32>(inst.rd, static_cast<s32>(result), i);
    }
}

void Interpreter::paddub() {
    for (int i = 0; i < 16; i++) {
        u16 result = static_cast<u16>(ctx.GetReg<u8>(inst.rs, i)) + static_cast<u16>(ctx.GetReg<u8>(inst.rt, i));
        if (result > 0xff) {
            result = 0xff;
        }
        ctx.SetReg<u8>(inst.rd, result & 0xff, i);
    }
}

void Interpreter::padduh() {
    for (int i = 0; i < 8; i++) {
        u32 result = static_cast<u32>(ctx.GetReg<u16>(inst.rs, i)) + static_cast<u32>(ctx.GetReg<u16>(inst.rt, i));
        if (result > 0xffff) {
            result = 0xffff;
        }
        ctx.SetReg<u16>(inst.rd, result & 0xffff, i);
    }
}

void Interpreter::padduw() {
    for (int i = 0; i < 4; i++) {
        u64 result = static_cast<u64>(ctx.GetReg<u32>(inst.rs, i)) + static_cast<u64>(ctx.GetReg<u32>(inst.rt, i));
        if (result > 0xffffffff) {
            result = 0xffffffff;
        }
        ctx.SetReg<u32>(inst.rd, result & 0xffffffff, i);
    }
}

void Interpreter::paddw() {
    for (int i = 0; i < 4; i++) {
        ctx.SetReg<s32>(inst.rd, ctx.GetReg<s32>(inst.rs, i) + ctx.GetReg<s32>(inst.rt, i), i);
    }
}

void Interpreter::padsbh() {
    for (int i = 0; i < 4; i++) {
        ctx.SetReg<s16>(inst.rd, ctx.GetReg<s16>(inst.rs, i) - ctx.GetReg<s16>(inst.rt, i), i);
        ctx.SetReg<s16>(inst.rd, ctx.GetReg<s16>(inst.rs, i + 4) + ctx.GetReg<s16>(inst.rt, i + 4), i + 4);
    }
}

void Interpreter::pmaxh() {
    for (int i = 0; i < 8; i++) {
        if (ctx.GetReg<s16>(inst.rs, i) > ctx.GetReg<s16>(inst.rt, i)) {
            ctx.SetReg<s16>(inst.rd, ctx.GetReg<s16>(inst.rs, i), i);
        } else {
            ctx.SetReg<s16>(inst.rd, ctx.GetReg<s16>(inst.rt, i), i);
        }
    }
}

void Interpreter::pmaxw() {
    for (int i = 0; i < 4; i++) {
        if (ctx.GetReg<s32>(inst.rs, i) > ctx.GetReg<s32>(inst.rt, i)) {
            ctx.SetReg<s32>(inst.rd, ctx.GetReg<s32>(inst.rs, i), i);
        } else {
            ctx.SetReg<s32>(inst.rd, ctx.GetReg<s32>(inst.rt, i), i);
        }
    }
}

void Interpreter::pminh() {
    for (int i = 0; i < 8; i++) {
        if (ctx.GetReg<s16>(inst.rs, i) > ctx.GetReg<s16>(inst.rt, i)) {
            ctx.SetReg<s16>(inst.rd, ctx.GetReg<s16>(inst.rt, i), i);
        } else {
            ctx.SetReg<s16>(inst.rd, ctx.GetReg<s16>(inst.rs, i), i);
        }
    }
}

void Interpreter::pminw() {
    for (int i = 0; i < 4; i++) {
        if (ctx.GetReg<s32>(inst.rs, i) > ctx.GetReg<s32>(inst.rt, i)) {
            ctx.SetReg<s32>(inst.rd, ctx.GetReg<s32>(inst.rt, i), i);
        } else {
            ctx.SetReg<s32>(inst.rd, ctx.GetReg<s32>(inst.rs, i), i);
        }
    }
}

void Interpreter::psubb() {
    for (int i = 0; i < 16; i++) {
        ctx.SetReg<s8>(inst.rd, ctx.GetReg<s8>(inst.rs, i) - ctx.GetReg<s8>(inst.rt, i), i);
    }
}

void Interpreter::psubh() {
    for (int i = 0; i < 8; i++) {
        ctx.SetReg<s16>(inst.rd, ctx.GetReg<s16>(inst.rs, i) - ctx.GetReg<s16>(inst.rt, i), i);
    }
}

void Interpreter::psubsb() {
    for (int i = 0; i < 16; i++) {
        s16 result = static_cast<s16>(ctx.GetReg<s8>(inst.rs, i)) - static_cast<s16>(ctx.GetReg<s8>(inst.rt, i));
        if (result > 0x7f) {
            result = 0x7f;
        } else if (result < -0x80) {
            result = -0x80;
        }
        ctx.SetReg<s8>(inst.rd, result & 0xff, i);
    }
}

void Interpreter::psubsh() {
    for (int i = 0; i < 8; i++) {
        s32 result = static_cast<s32>(ctx.GetReg<s16>(inst.rs, i)) - static_cast<s32>(ctx.GetReg<s16>(inst.rt, i));
        if (result > 0x7fff) {
            result = 0x7fff;
        } else if (result < -0x8000) {
            result = -0x8000;
        }
        ctx.SetReg<s16>(inst.rd, result & 0xffff, i);
    }
}

void Interpreter::psubsw() {
    for (int i = 0; i < 4; i++) {
        s64 result = static_cast<s64>(ctx.GetReg<s32>(inst.rs, i)) - static_cast<s64>(ctx.GetReg<s32>(inst.rt, i));
        if (result > 0x7fffffff) {
            result = 0x7fffffff;
        } else if (result < static_cast<s32>(0x80000000)) {
            result = static_cast<s32>(0x80000000);
        }
        ctx.SetReg<s32>(inst.rd, static_cast<s32>(result), i);
    }
}

void Interpreter::psubub() {
    for (int i = 0; i < 16; i++) {
        u16 result = static_cast<u16>(ctx.GetReg<u8>(inst.rs, i)) - static_cast<u16>(ctx.GetReg<u8>(inst.rt, i));
        if (result > 0xff) {
            result = 0;
        }
        ctx.SetReg<u8>(inst.rd, result & 0xff, i);
    }
}

void Interpreter::psubuh() {
    for (int i = 0; i < 8; i++) {
        u32 result = static_cast<u32>(ctx.GetReg<u16>(inst.rs, i)) - static_cast<u32>(ctx.GetReg<u16>(inst.rt, i));
        if (result > 0xffff) {
            result = 0;
        }
        ctx.SetReg<u16>(inst.rd, result & 0xffff, i);
    }
}

void Interpreter::psubuw() {
    for (int i = 0; i < 4; i++) {
        u64 result = static_cast<u64>(ctx.GetReg<u32>(inst.rs, i)) - static_cast<u64>(ctx.GetReg<u32>(inst.rt, i));
        if (result > 0xffffffff) {
            result = 0;
        }
        ctx.SetReg<u32>(inst.rd, result & 0xffffffff, i);
    }
}

void Interpreter::psubw() {
    for (int i = 0; i < 4; i++) {
        ctx.SetReg<s32>(inst.rd, ctx.GetReg<s32>(inst.rs, i) - ctx.GetReg<s32>(inst.rt, i), i);
    }
}

void Interpreter::pceqb() {
    for (int i = 0; i < 16; i++) {
        if (ctx.GetReg<u8>(inst.rs, i) == ctx.GetReg<u8>(inst.rt, i)) {
            ctx.SetReg<u8>(inst.rd, 0xff, i);
        } else {
            ctx.SetReg<u8>(inst.rd, 0, i);
        }
    }
}

void Interpreter::pceqh() {
    for (int i = 0; i < 8; i++) {
        if (ctx.GetReg<u16>(inst.rs, i) == ctx.GetReg<u16>(inst.rt, i)) {
            ctx.SetReg<u16>(inst.rd, 0xffff, i);
        } else {
            ctx.SetReg<u16>(inst.rd, 0, i);
        }
    }
}

void Interpreter::pceqw() {
    for (int i = 0; i < 4; i++) {
        if (ctx.GetReg<u32>(inst.rs, i) == ctx.GetReg<u32>(inst.rt, i)) {
            ctx.SetReg<u32>(inst.rd, 0xffffffff, i);
        } else {
            ctx.SetReg<u32>(inst.rd, 0, i);
        }
    }
}

void Interpreter::pcgtb() {
    for (int i = 0; i < 16; i++) {
        if (ctx.GetReg<s8>(inst.rs, i) > ctx.GetReg<s8>(inst.rt, i)) {
            ctx.SetReg<u8>(inst.rd, 0xff, i);
        } else {
            ctx.SetReg<u8>(inst.rd, 0, i);
        }
    }
}

void Interpreter::pcgth() {
    for (int i = 0; i < 8; i++) {
        if (ctx.GetReg<s16>(inst.rs, i) > ctx.GetReg<s16>(inst.rt, i)) {
            ctx.SetReg<u16>(inst.rd, 0xffff, i);
        } else {
            ctx.SetReg<u16>(inst.rd, 0, i);
        }
    }
}

void Interpreter::pcgtw() {
    for (int i = 0; i < 4; i++) {
        if (ctx.GetReg<s32>(inst.rs, i) > ctx.GetReg<s32>(inst.rt, i)) {
            ctx.SetReg<u32>(inst.rd, 0xffffffff, i);
        } else {
            ctx.SetReg<u32>(inst.rd, 0, i);
        }
    }
}

// primary instructions
void Interpreter::slti() {
    ctx.SetReg<u64>(inst.rt, ctx.GetReg<s64>(inst.rs) < common::SignExtend<s64, 16>(inst.imm));
}

void Interpreter::bne() {
    Branch(ctx.GetReg<u64>(inst.rs) != ctx.GetReg<u64>(inst.rt));
}

void Interpreter::lui() {
    ctx.SetReg<u64>(inst.rt, common::SignExtend<s64, 32>(inst.imm << 16));
}

void Interpreter::ori() {
    ctx.SetReg<u64>(inst.rt, ctx.GetReg<u64>(inst.rs) | inst.imm);
}

void Interpreter::addiu() {
    s32 result = ctx.GetReg<s64>(inst.rs) + (s16)inst.imm;
    ctx.SetReg<s64>(inst.rt, result);
}

void Interpreter::sw() {
    ctx.write<u32>(ctx.GetReg<u32>(inst.rs) + inst.simm, ctx.GetReg<u32>(inst.rt));
}

void Interpreter::sd() {
    ctx.write<u64>(ctx.GetReg<u32>(inst.rs) + inst.simm, ctx.GetReg<u64>(inst.rt));
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
    ctx.SetReg<s64>(inst.rt, static_cast<s8>(ctx.read<u8>(ctx.GetReg<u32>(inst.rs) + inst.simm)));
}

void Interpreter::lbu() {
    ctx.SetReg<u64>(inst.rt, ctx.read<u8>(ctx.GetReg<u32>(inst.rs) + inst.simm));
}

void Interpreter::ld() {
    ctx.SetReg<u64>(inst.rt, ctx.read<u64>(ctx.GetReg<u32>(inst.rs) + inst.simm));
}

void Interpreter::j() {
    Jump(((ctx.pc + 4) & 0xF0000000) + (inst.offset << 2));
}

void Interpreter::lw() {
    ctx.SetReg<s64>(inst.rt, (s32)ctx.read<u32>(ctx.GetReg<u32>(inst.rs) + inst.simm));
}

void Interpreter::sb() {
    ctx.write<u8>(ctx.GetReg<u32>(inst.rs) + inst.simm, ctx.GetReg<u8>(inst.rt));
}

void Interpreter::blez() {
    Branch(ctx.GetReg<s64>(inst.rs) <= 0);
}

void Interpreter::lhu() {
    ctx.SetReg<u64>(inst.rt, ctx.read<u16>(ctx.GetReg<u32>(inst.rs) + inst.simm));
}

void Interpreter::bgtz() {
    Branch(ctx.GetReg<s64>(inst.rs) > 0);
}

void Interpreter::sh() {
    ctx.write<u16>(ctx.GetReg<u32>(inst.rs) + inst.simm, ctx.GetReg<u16>(inst.rt));
}

void Interpreter::xori() {
    ctx.SetReg<u64>(inst.rt, ctx.GetReg<u64>(inst.rs) ^ inst.imm);
}

void Interpreter::daddiu() {
    ctx.SetReg<s64>(inst.rt, ctx.GetReg<s64>(inst.rs) + common::SignExtend<s64, 16>(inst.imm));
}

void Interpreter::sq() {
    u128 reg = ctx.GetReg<u128>(inst.rt);
    u32 addr = (ctx.GetReg<u32>(inst.rs) + inst.simm) & ~0xf;
    ctx.write<u128>(addr, reg);
}

void Interpreter::lq() {
    u32 vaddr = (ctx.GetReg<u32>(inst.rs) + inst.simm) & ~0xf;
    u128 data = ctx.read<u128>(vaddr);
    ctx.SetReg<u128>(inst.rt, data);
}

void Interpreter::lh() {
    ctx.SetReg<s64>(inst.rt, (s16)ctx.read<u16>(ctx.GetReg<u32>(inst.rs) + inst.simm));
}

void Interpreter::cache() {
    // handle later
}

void Interpreter::lwu() {
    ctx.SetReg<u64>(inst.rt, ctx.read<u32>(ctx.GetReg<u32>(inst.rs) + inst.simm));
}

void Interpreter::ldl() {
    const u64 mask[8] = {
        0x00FFFFFFFFFFFFFF, 0x0000FFFFFFFFFFFF, 0x000000FFFFFFFFFF, 0x00000000FFFFFFFF,
        0x0000000000FFFFFF, 0x000000000000FFFF, 0x00000000000000FF, 0x0000000000000000,
    };

    const u8 shift[8] = {56, 48, 40, 32, 24, 16, 8, 0};

    u32 addr = ctx.GetReg<u32>(inst.rs) + inst.simm;
    int index = addr & 0x7;
    u64 data = ctx.read<u64>(addr & ~0x7);
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
    u64 data = ctx.read<u64>(addr & ~0x7);
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

    u64 data = ctx.read<u64>(addr & ~0x7);
    u64 reg = ctx.GetReg<u64>(inst.rt);
    ctx.write<u64>(addr & ~0x7, (reg >> shift[index]) | (data & mask[index]));
}

void Interpreter::sdr() {
    const u64 mask[8] = {
        0x0000000000000000, 0x00000000000000ff, 0x000000000000ffff, 0x0000000000ffffff,
        0x00000000ffffffff, 0x000000ffffffffff, 0x0000ffffffffffff, 0x00ffffffffffffff,
    };

    const u8 shift[8] = {0, 8, 16, 24, 32, 40, 48, 56};

    u32 addr = ctx.GetReg<u32>(inst.rs) + inst.simm;
    int index = addr & 0x7;

    u64 data = ctx.read<u64>(addr & ~0x7);
    u64 reg = ctx.GetReg<u64>(inst.rt);
    ctx.write<u64>(addr & ~0x7, (reg << shift[index]) | (data & mask[index]));
}

void Interpreter::bgtzl() {
    BranchLikely(ctx.GetReg<s64>(inst.rs) > 0);
}

void Interpreter::blezl() {
    BranchLikely(ctx.GetReg<s64>(inst.rs) <= 0);
}

void Interpreter::lwl() {
    u32 addr = ctx.GetReg<u32>(inst.rs) + inst.simm;
    u32 aligned_data = ctx.read<u32>(addr & ~0x3);
    u8 shift = (addr & 0x3) * 8;
    u32 mask = 0xffffff >> shift;
    ctx.SetReg<s64>(inst.rt, static_cast<s32>((ctx.GetReg<u32>(inst.rt) & mask) | (aligned_data << (24 - shift))));
}

void Interpreter::lwr() {
    u32 addr = ctx.GetReg<u32>(inst.rs) + inst.simm;
    u32 aligned_data = ctx.read<u32>(addr & ~0x3);
    u8 shift = (addr & 0x3) * 8;
    u32 mask = 0xffffff00 << (24 - shift);
    u32 updated_data = (ctx.GetReg<u32>(inst.rt) & mask) | (aligned_data >> shift);

    // sign extend to 64-bit bits when shift is 0
    if (shift) {
        ctx.SetReg<u32>(inst.rt, updated_data);
    } else {
        ctx.SetReg<s64>(inst.rt, static_cast<s32>(updated_data));
    }
}

void Interpreter::swl() {
    u32 addr = ctx.GetReg<u32>(inst.rs) + inst.simm;
    u32 aligned_data = ctx.read<u32>(addr & ~0x3);
    u8 shift = (addr & 0x3) * 8;
    u32 mask = 0xffffff00 << shift;
    u32 updated_data = (aligned_data & mask) | (ctx.GetReg<u32>(inst.rt) >> (24 - shift));
    ctx.write<u32>(addr & ~0x3, updated_data);
}

void Interpreter::swr() {
    u32 addr = ctx.GetReg<u32>(inst.rs) + inst.simm;
    u32 aligned_data = ctx.read<u32>(addr & ~0x3);
    u8 shift = (addr & 0x3) * 8;
    u32 mask = 0xffffff >> (24 - shift);
    u32 updated_data = (aligned_data & mask) | (ctx.GetReg<u32>(inst.rt) << shift);
    ctx.write<u32>(addr & ~0x3, updated_data);
}

void Interpreter::pref() {
    // prefetch
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
    Branch(ctx.GetReg<s64>(inst.rs) >= 0);
}

void Interpreter::bgezall() {
    ctx.SetReg<u64>(31, ctx.pc + 8);
    BranchLikely(ctx.GetReg<s64>(inst.rs) >= 0);
}

void Interpreter::bltzal() {
    ctx.SetReg<u64>(31, ctx.pc + 8);
    Branch(ctx.GetReg<s64>(inst.rs) < 0);
}

void Interpreter::bltzall() {
    ctx.SetReg<u64>(31, ctx.pc + 8);
    BranchLikely(ctx.GetReg<s64>(inst.rs) < 0);
}

// secondary instructions
void Interpreter::sll() {
    u32 result = ctx.GetReg<u32>(inst.rt) << inst.imm5;
    ctx.SetReg<s64>(inst.rd, static_cast<s32>(result));
}

void Interpreter::jr() {
    Jump(ctx.GetReg<u32>(inst.rs));
}

void Interpreter::sync() {
    
}

void Interpreter::jalr() {
    u32 return_addr = ctx.pc + 8;
    Jump(ctx.GetReg<u32>(inst.rs));
    ctx.SetReg<u64>(inst.rd, return_addr);
}

void Interpreter::daddu() {
    ctx.SetReg<u64>(inst.rd, ctx.GetReg<s64>(inst.rs) + ctx.GetReg<s64>(inst.rt));
}

void Interpreter::orr() {
    ctx.SetReg<u64>(inst.rd, ctx.GetReg<u64>(inst.rs) | ctx.GetReg<u64>(inst.rt));
}

void Interpreter::mult() {
    s64 result = static_cast<s64>(ctx.GetReg<s32>(inst.rs)) * static_cast<s64>(ctx.GetReg<s32>(inst.rt));
    ctx.lo = common::SignExtend<s64, 32>(result & 0xffffffff);
    ctx.hi = common::SignExtend<s64, 32>(result >> 32);
    ctx.SetReg<u64>(inst.rd, ctx.lo);
}

void Interpreter::divu() {
    if (ctx.GetReg<u32>(inst.rt) == 0) {
        ctx.lo = 0xFFFFFFFFFFFFFFFF;
        ctx.hi = common::SignExtend<s64, 32>(ctx.GetReg<u32>(inst.rs));
    } else {
        ctx.lo = common::SignExtend<s64, 32>(ctx.GetReg<u32>(inst.rs) / ctx.GetReg<u32>(inst.rt));
        ctx.hi = common::SignExtend<s64, 32>(ctx.GetReg<u32>(inst.rs) % ctx.GetReg<u32>(inst.rt));
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
    if ((ctx.GetReg<s32>(inst.rs) == static_cast<s32>(0x80000000)) && (ctx.GetReg<s32>(inst.rt) == -1)) {
        ctx.lo = common::SignExtend<s64, 32>(0x80000000);
        ctx.hi = 0;
    } else if (ctx.GetReg<s32>(inst.rt) == 0) {
        if (ctx.GetReg<s32>(inst.rs) >= 0) {
            ctx.lo = common::SignExtend<s64, 32>(0xffffffff);
        } else {
            ctx.lo = 1;
        }
        ctx.hi = common::SignExtend<s64, 32>(ctx.GetReg<s32>(inst.rs));
    } else {
        ctx.lo = common::SignExtend<s64, 32>(ctx.GetReg<s32>(inst.rs) / ctx.GetReg<s32>(inst.rt));
        ctx.hi = common::SignExtend<s64, 32>(ctx.GetReg<s32>(inst.rs) % ctx.GetReg<s32>(inst.rt));
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
    ctx.SetReg<s64>(inst.rd, common::SignExtend<s64, 32>(result));
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
    u8 opcode = ctx.read<u8>(ctx.pc - 4);

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

void Interpreter::multu() {
    u64 result = static_cast<u64>(ctx.GetReg<u32>(inst.rs)) * static_cast<u64>(ctx.GetReg<u32>(inst.rt));
    ctx.lo = common::SignExtend<s64, 32>(result & 0xffffffff);
    ctx.hi = common::SignExtend<s64, 32>(result >> 32);
    ctx.SetReg<u64>(inst.rd, ctx.lo);
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
    common::Error("[ee::Interpreter] %s = %08x at %08x (primary = %d, secondary = %d, regimm = %d, rs = %d, imm5 = %d) is undefined", DisassembleInstruction(inst, ctx.pc).c_str(), inst.data, ctx.pc, inst.opcode, inst.func, inst.rt, inst.rs, inst.imm5);
}

void Interpreter::stub_instruction() {
    common::Log("[ee::Interpreter] %s = %08x at %08x (primary = %d, secondary = %d, regimm = %d, rs = %d, imm5 = %d) is undefined", DisassembleInstruction(inst, ctx.pc).c_str(), inst.data, ctx.pc, inst.opcode, inst.func, inst.rt, inst.rs, inst.imm5);
}

} // namespace ee