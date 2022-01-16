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

void IOPCore::EnableHardwareInterrupt() {
    log_fatal("handle")
}

void IOPCore::DisableHardwareInterrupt() {
    cop0.gpr[13] &= ~(1 << 10);
}