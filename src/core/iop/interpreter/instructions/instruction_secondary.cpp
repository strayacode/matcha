#include <common/types.h>
#include <core/iop/interpreter/interpreter.h>

void IOPInterpreter::sll() {
    SetReg(inst.rd, GetReg(inst.rt) << inst.imm5);
}

void IOPInterpreter::jr() {
    regs.next_pc = GetReg(inst.rs);
    branch_delay = true;
}

void IOPInterpreter::orr() {
    SetReg(inst.rd, GetReg(inst.rs) | GetReg(inst.rt));
}

void IOPInterpreter::addu() {
    SetReg(inst.rd, GetReg(inst.rs) + GetReg(inst.rt));
}

void IOPInterpreter::sltu() {
    SetReg(inst.rd, GetReg(inst.rs) < GetReg(inst.rt));
}

void IOPInterpreter::andd() {
    SetReg(inst.rd, GetReg(inst.rs) & GetReg(inst.rt));
}

void IOPInterpreter::slt() {
    SetReg(inst.rd, (s32)GetReg(inst.rs) < (s32)GetReg(inst.rt));
}

void IOPInterpreter::sra() {
    SetReg(inst.rd, (s32)GetReg(inst.rt) >> inst.imm5);
}

void IOPInterpreter::srl() {
    SetReg(inst.rd, GetReg(inst.rt) >> inst.imm5);
}

void IOPInterpreter::subu() {
    SetReg(inst.rd, GetReg(inst.rs) - GetReg(inst.rt));
}

void IOPInterpreter::divu() {
    if (GetReg(inst.rt) == 0) {
        regs.lo = 0xFFFFFFFF;
        regs.hi = GetReg(inst.rs);
    } else {
        regs.lo = GetReg(inst.rs) / GetReg(inst.rt);
        regs.hi = GetReg(inst.rs) % GetReg(inst.rt);
    }
}

void IOPInterpreter::mflo() {
    SetReg(inst.rd, regs.lo);
}

void IOPInterpreter::jalr() {
    u32 addr = GetReg(inst.rs);

    SetReg(inst.rd, regs.pc + 8);
    
    regs.next_pc = addr;
    branch_delay = true;
}

void IOPInterpreter::xorr() {
    SetReg(inst.rd, GetReg(inst.rs) ^ GetReg(inst.rt));
}

void IOPInterpreter::sllv() {
    SetReg(inst.rd, GetReg(inst.rt) << (GetReg(inst.rs) & 0x1F));
}

void IOPInterpreter::mfhi() {
    SetReg(inst.rd, regs.hi);
}

void IOPInterpreter::multu() {
    u64 result = (u64)GetReg(inst.rs) * (u64)GetReg(inst.rt);

    regs.lo = result & 0xFFFFFFFF;
    regs.hi = result >> 32;
}

void IOPInterpreter::mthi() {
    regs.hi = GetReg(inst.rs);
}

void IOPInterpreter::mtlo() {
    regs.lo = GetReg(inst.rs);
}

void IOPInterpreter::syscall_exception() {
    DoException(ExceptionType::Syscall);
}

void IOPInterpreter::mult() {
    s64 result = (s64)(s32)GetReg(inst.rs) * (s64)(s32)GetReg(inst.rt);

    regs.lo = result & 0xFFFFFFFF;
    regs.hi = result >> 32;
}

void IOPInterpreter::nor() {
    SetReg(inst.rd, 0xFFFFFFFF ^ (GetReg(inst.rs) | GetReg(inst.rt)));
}

void IOPInterpreter::srlv() {
    SetReg(inst.rd, GetReg(inst.rt) >> (GetReg(inst.rs) & 0x1F));
}

void IOPInterpreter::add() {
    SetReg(inst.rd, GetReg(inst.rs) + GetReg(inst.rt));
}

void IOPInterpreter::div() {
    s32 rs = (s32)GetReg(inst.rs);
    s32 rt = (s32)GetReg(inst.rt);

    if (rt == 0) {
        if (rs < 0) {
            regs.lo = 1;
        } else {
            regs.lo = 0xFFFFFFFF;
        }

        regs.hi = rs;
    } else if ((u32)rs == 0x80000000 && (u32)rt == 0xFFFFFFFF) {
        regs.lo = 0x80000000;
        regs.hi = 0;
    } else {
        regs.lo = rs / rt;
        regs.hi = rs % rt;
    }
}

void IOPInterpreter::srav() {
    u8 shift_amount = GetReg(inst.rs) & 0x1F;

    SetReg(inst.rd, (s32)GetReg(inst.rt) >> shift_amount);
}