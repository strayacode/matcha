#include "common/log.h"
#include "core/iop/context.h"
#include "core/system.h"

namespace iop {

Context::Context(System& system) : intc(*this), interpreter(*this), system(system) {}

void Context::Reset() {
    gpr.fill(0);
    pc = 0xbfc00000;
    npc = 0;
    hi = 0;
    lo = 0;
    
    cop0.Reset();
    cdvd.Reset();
    intc.Reset();
    interpreter.Reset();

    // TODO: add vtlb mappings
}

void Context::Run(int cycles) {
    interpreter.Run(cycles);
}

u8 Context::ReadByte(u32 addr) {
    return system.memory.IOPRead<u8>(addr);
}

u16 Context::ReadHalf(u32 addr) {
    return system.memory.IOPRead<u16>(addr);
}

u32 Context::ReadWord(u32 addr) {
    return system.memory.IOPRead<u32>(addr);
}

void Context::WriteByte(u32 addr, u8 value) {
    system.memory.IOPWrite<u8>(addr, value);
}

void Context::WriteHalf(u32 addr, u16 value) {
    system.memory.IOPWrite<u16>(addr, value);
}

void Context::WriteWord(u32 addr, u32 value) {
    system.memory.IOPWrite<u32>(addr, value);
}

void Context::RaiseInterrupt(bool value) {
    interpreter.RaiseInterrupt(value);
    if (value) {
        cop0.cause.data |= 1 << 10;
    } else {
        cop0.cause.data &= ~(1 << 10);
    }
}

} // namespace iop