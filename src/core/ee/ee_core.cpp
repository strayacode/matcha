#include "core/ee/ee_core.h"
#include "core/system.h"

EECore::EECore(System& system) : system(system) {}

void EECore::Reset() {
    for (int i = 0; i < 512; i++) {
        regs.gpr[i] = 0;
    }

    regs.pc = 0xBFC00000;
    regs.next_pc = 0;
    regs.hi = 0;
    regs.lo = 0;
    regs.hi1 = 0;
    regs.lo1 = 0;
    regs.sa = 0;
    inst.data = 0;
    branch_delay = false;
    branch = false;

    cop0.Reset();
    cop1.Reset();
    interpreter_table.Generate();
}

void EECore::Run(int cycles) {
    while (cycles--) {
        inst = CPUInstruction{ReadWord(regs.pc)};

        interpreter_table.Execute(*this, inst);

        regs.pc += 4;

        if (branch_delay) {
            if (branch) {
                regs.pc = regs.next_pc;
                branch_delay = false;
                branch = false;
            } else {
                branch = true;
            }
        }

        cop0.CountUp();
    }
}

u8 EECore::ReadByte(u32 addr) {
    return system.memory.EERead<u8>(addr);
}

u16 EECore::ReadHalf(u32 addr) {
    return system.memory.EERead<u16>(addr);
}

u32 EECore::ReadWord(u32 addr) {
    return system.memory.EERead<u32>(addr);
}

u64 EECore::ReadDouble(u32 addr) {
    return system.memory.EERead<u64>(addr);
}

void EECore::WriteByte(u32 addr, u8 data) {
    system.memory.EEWrite<u8>(addr, data);
}

void EECore::WriteHalf(u32 addr, u16 data) {
    system.memory.EEWrite<u16>(addr, data);
}

void EECore::WriteWord(u32 addr, u32 data) {
    system.memory.EEWrite<u32>(addr, data);
}

void EECore::WriteDouble(u32 addr, u64 data) {
    system.memory.EEWrite<u64>(addr, data);
}

void EECore::WriteQuad(u32 addr, u128 data) {
    system.memory.EEWrite<u128>(addr, data);
}

void EECore::DoException(u32 target, ExceptionType exception) {
    u32 status = cop0.GetReg(12);
    u32 cause = cop0.GetReg(13);

    bool level2_exception = static_cast<int>(exception) >= 14;
    int code = level2_exception ? static_cast<int>(exception) - 14 : static_cast<int>(exception);

    if (level2_exception) {
        log_fatal("handle level 2 exception");
    } else {
        cause |= (code << 2);
        if (branch_delay) {
            cop0.SetReg(14, regs.pc - 4);
            cause |= (1 << 31);
        } else {
            cop0.SetReg(14, regs.pc);
            cause &= ~(1 << 31);
        }

        status |= (1 << 1);
        regs.pc = target - 4;
    }

    cop0.SetReg(12, status);
    cop0.SetReg(13, cause);
}