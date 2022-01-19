#include "core/iop/cpu_core.h"
#include "core/system.h"

IOPCore::IOPCore(System* system) : system(system), interrupt_controller(*this) {

}

u8 IOPCore::ReadByte(u32 addr) {
    return system->memory.IOPRead<u8>(addr);
}

u16 IOPCore::ReadHalf(u32 addr) {
    return system->memory.IOPRead<u16>(addr);
}

u32 IOPCore::ReadWord(u32 addr) {
    return system->memory.IOPRead<u32>(addr);
}

void IOPCore::WriteByte(u32 addr, u8 data) {
    system->memory.IOPWrite<u8>(addr, data);
}

void IOPCore::WriteHalf(u32 addr, u16 data) {
    system->memory.IOPWrite<u16>(addr, data);
}

void IOPCore::WriteWord(u32 addr, u32 data) {
    system->memory.IOPWrite<u32>(addr, data);
}

void IOPCore::SendInterruptSignal(bool value) {
    if (value) {
        cop0.gpr[13] |= (1 << 10);
    } else {
        cop0.gpr[13] &= ~(1 << 10);
    }
}

void IOPCore::CheckInterrupts() {
    bool iec = cop0.gpr[12] & 0x1;
    u8 im = (cop0.gpr[12] >> 8) & 0xFF;
    u8 ip = (cop0.gpr[13] >> 8) & 0xFF;

    if (iec && (im & ip)) {
        DoException(ExceptionType::Interrupt);
    }
}

void IOPCore::DoException(ExceptionType exception) {
    // store the address where the exception took place
    // in cop0 epc
    // if we are currently in a branch delay slot with a syscall
    // instruction then we save the address of the branch instruction
    // (pc - 4)
    if (branch_delay) {
        cop0.SetReg(14, regs.pc - 4);
    } else {
        cop0.SetReg(14, regs.pc);
    }

    // record the cause of the exception (in this case a syscall)
    cop0.SetReg(13, static_cast<u8>(exception) << 2);

    u32 exception_base = 0;

    if (cop0.GetReg(12) & (1 << 22)) {
        // exception base address in rom/kseg1
        exception_base = 0xBFC00180;
    } else {
        // exception base address in rom/kseg0
        exception_base = 0x80000080;
    }

    // shift the interrupt and kernel/user mode bit 
    // by 2 bits to the left to act as a stack with maximum of 3 entries
    u8 stack = cop0.gpr[12] & 0x3F;
    cop0.gpr[12] &= ~0x3F;
    cop0.gpr[12] |= (stack << 2) & 0x3F;

    // since we increment by 4 after each instruction we need to account for that
    // so that we can execute at the exception base on the next instruction
    regs.pc = exception_base - 4;
}