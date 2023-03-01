#include "core/ee/intc.h"
#include "core/system.h"

EEINTC::EEINTC(System& system) : system(system) {}

void EEINTC::Reset() {
    mask = 0;
    stat = 0;
}

u16 EEINTC::ReadMask() {
    common::Log("[INTC] read mask %04x", mask);
    return mask;
}

// a bit set to 1 in stat means
// an irq was raised
u16 EEINTC::ReadStat() {
    common::Log("[INTC] read stat %04x", stat);
    return stat;
}

// writing 1 to mask reverses a bit
// while writing 0 has no effect
void EEINTC::WriteMask(u16 data) {
    common::Log("[INTC] write mask %04x", data);
    mask ^= (data & 0x7FFF);
    CheckInterrupts();
}

// writing the bit 1 to stat clears an interrupt
// while writing 0 has no effect
void EEINTC::WriteStat(u16 data) {
    common::Log("[INTC] write stat %04x", data);
    stat &= ~(data & 0x7FFF);
    CheckInterrupts();
}

// when stat & mask is true, an int0 signal is assert into Cause.10. When Status.10 is true, an interrupt occurs and the ee jumps to 0x80000200 
void EEINTC::CheckInterrupts() {
    system.ee.RaiseInterrupt(0, stat & mask);
}

void EEINTC::RequestInterrupt(EEInterruptSource interrupt) {
    stat |= (1 << static_cast<int>(interrupt));
    CheckInterrupts();
}