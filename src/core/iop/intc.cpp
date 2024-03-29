#include "common/log.h"
#include "core/iop/intc.h"
#include "core/iop/context.h"

namespace iop {

INTC::INTC(Context& ctx) : ctx(ctx) {}

void INTC::Reset() {
    interrupt_mask = 0;
    interrupt_status = 0;
    interrupt_control = 0;
}

u32 INTC::ReadRegister(int offset) {
    switch (offset) {
    case 0x1f801070:
        return interrupt_status;
    case 0x1f801074:
        return interrupt_mask;
    case 0x1f801078: {
        u32 value = interrupt_control;
        interrupt_control = 0;
        UpdateInterrupts();
        return value;
    }
    default:
        common::Error("InterruptController: handle read offset %02x", offset);
    }

    return 0;
}

void INTC::WriteRegister(int offset, u32 data) {
    switch (offset) {
    case 0x1f801070:
        // common::Log("[IOP INTC] I_STAT write %08x", data);
        interrupt_status &= data & WRITE_MASK;
        UpdateInterrupts();
        break;
    case 0x1f801074:
        // common::Log("[IOP INTC] I_MASK write %08x", data);
        interrupt_mask = data & WRITE_MASK;
        UpdateInterrupts();
        break;
    case 0x1f801078:
        interrupt_control = data & 0x1;
        UpdateInterrupts();
        break;
    default:
        common::Error("InterruptController: handle write offset %02x", offset);
    }
}

void INTC::RequestInterrupt(InterruptSource source) {
    interrupt_status |= 1 << static_cast<u32>(source);
    UpdateInterrupts();
}

void INTC::UpdateInterrupts() {
    ctx.RaiseInterrupt(interrupt_control && (interrupt_status & interrupt_mask));
}

} // namespace iop