#include "core/ee/intc.h"
#include "core/ee/context.h"

namespace ee {

INTC::INTC(Context& ee) : ee(ee) {}

void INTC::Reset() {
    mask = 0;
    stat = 0;
}

u16 INTC::ReadMask() {
    common::Log("[ee::INTC] read mask %04x", mask);
    return mask;
}

// a bit set to 1 in stat means
// an irq was raised
u16 INTC::ReadStat() {
    common::Log("[ee::INTC] read stat %04x", stat);
    return stat;
}

// writing 1 to mask reverses a bit
// while writing 0 has no effect
void INTC::WriteMask(u16 data) {
    common::Log("[ee::INTC] write mask %04x", data);
    mask ^= (data & 0x7FFF);
    CheckInterrupts();
}

// writing the bit 1 to stat clears an interrupt
// while writing 0 has no effect
void INTC::WriteStat(u16 data) {
    common::Log("[ee::INTC] write stat %04x", data);
    stat &= ~(data & 0x7FFF);
    CheckInterrupts();
}

// when stat & mask is true, an int0 signal is assert into Cause.10. When Status.10 is true, an interrupt occurs and the ee jumps to 0x80000200 
void INTC::CheckInterrupts() {
    ee.RaiseInterrupt(0, stat & mask);
}

void INTC::RequestInterrupt(InterruptSource interrupt) {
    stat |= (1 << static_cast<int>(interrupt));
    CheckInterrupts();
}

} // namespace ee