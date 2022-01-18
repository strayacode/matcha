#include <assert.h>
#include "core/ee/ee_core.h"
#include "core/system.h"
#include "core/ee/disassembler.h"

EECore::EECore(System& system) : system(system) {}

void EECore::Reset() {
    for (int i = 0; i < 512; i++) {
        gpr[i] = 0;
    }

    pc = 0xBFC00000;
    next_pc = 0;
    hi = 0;
    lo = 0;
    hi1 = 0;
    lo1 = 0;
    sa = 0;
    inst.data = 0;
    branch_delay = false;
    branch = false;

    cop0.Reset();
    cop1.Reset();
    interpreter_table.Generate();
}

bool disassemble = false;

void EECore::Run(int cycles) {
    while (cycles--) {
        inst = CPUInstruction{ReadWord(pc)};

        if (pc == 0x82000) {
            disassemble = true;
        }

        if (disassemble) {
            fprintf(fp, "%08x %08x %s\n", pc, inst.data, EEDisassembleInstruction(inst, pc).c_str());
        }

        // if (print_regs) {
        //     PrintRegs();
        // }

        interpreter_table.Execute(*this, inst);

        pc += 4;

        if (branch_delay) {
            if (branch) {
                pc = next_pc;
                branch_delay = false;
                branch = false;
            } else {
                branch = true;
            }
        }

        cop0.CountUp();
        CheckInterrupts();
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
            cop0.SetReg(14, pc - 4);
            cause |= (1 << 31);
        } else {
            cop0.SetReg(14, pc);
            cause &= ~(1 << 31);
        }

        status |= (1 << 1);
        pc = target - 4;
    }

    cop0.SetReg(12, status);
    cop0.SetReg(13, cause);
}

void EECore::SendInterruptSignal(int signal, bool value) {
    if (value) {
        cop0.gpr[13] |= (1 << (10 + signal));
    } else {
        cop0.gpr[13] &= ~(1 << (10 + signal));
    }
}

void EECore::CheckInterrupts() {
    if (InterruptsEnabled()) {
        bool int0_enable = (cop0.gpr[12] >> 10) & 0x1;
        bool int0_pending = (cop0.gpr[13] >> 10) & 0x1;
        bool timer_enable = (cop0.gpr[12] >> 15) & 0x1;

        assert(timer_enable == false);
        
        if (int0_enable && int0_pending) {
            DoException(0x80000200, ExceptionType::Interrupt);
            return;
        }

        bool int1_enable = (cop0.gpr[12] >> 11) & 0x1;
        bool int1_pending = (cop0.gpr[13] >> 11) & 0x1;

        if (int1_enable && int1_pending) {
            log_fatal("handle");
            return;
        }

        // TODO: handle cop0 compare interrupts
    }
}

bool EECore::InterruptsEnabled() {
    bool ie = cop0.gpr[12] & 0x1;
    bool eie = (cop0.gpr[12] >> 16) & 0x1;
    bool exl = (cop0.gpr[12] >> 1) & 0x1;
    bool erl = (cop0.gpr[12] >> 2) & 0x1;

    return ie && eie && !exl && !erl;
}

void EECore::PrintRegs() {
    for (int i = 0; i < 32; i++) {
        fprintf(fp, "%s: %016lx%016lx\n", EEGetRegisterName(i).c_str(), GetReg<u128>(i).i.hi, GetReg<u128>(i).i.lo);
    }
}