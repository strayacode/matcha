#include "common/log.h"
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
        cop0.cause.data |= 1 << 10;
    } else {
        cop0.cause.data &= ~(1 << 10);
    }
}

void IOPCore::CheckInterrupts() {
    if (cop0.status.iec && (cop0.status.im & cop0.cause.ip)) {
        DoException(Exception::Interrupt);
    }
}

void IOPCore::DoException(Exception exception) {
    common::Log("[IOP] trigger exception with type %02x", static_cast<int>(exception));

    // record the cause of the exception
    cop0.cause.excode = static_cast<u8>(exception);

    // store pc to epc
    if (branch_delay) {
        cop0.epc = regs.pc - 4;
        cop0.cause.bd = true;
    } else {
        cop0.epc = regs.pc;
        cop0.cause.bd = false;
    }

    if (cop0.status.bev) {
        common::Log("[iop::Interpreter] handle bev=1");
    }

    u32 target = 0x80000080;

    // shift the interrupt and kernel/user mode bit 
    // by 2 bits to the left to act as a stack with maximum of 3 entries
    u8 stack = cop0.status.data & 0x3f;
    cop0.status.data &= ~0x3f;
    cop0.status.data |= (stack << 2) & 0x3f;

    // since we increment by 4 after each instruction we need to account for that
    // so that we can execute at the exception base on the next instruction
    regs.pc = target - 4;

    branch_delay = false;
    branch = false;
}