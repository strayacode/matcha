#include "common/log.h"
#include "core/iop/interrupt_controller.h"
#include "core/system.h"

IOPInterruptController::IOPInterruptController(IOPCore& cpu) : cpu(cpu) {}

void IOPInterruptController::Reset() {
    interrupt_mask = 0;
    interrupt_status = 0;
    interrupt_control = 0;
}

u32 IOPInterruptController::ReadRegister(int offset) {
    switch (offset) {
    case 0x0:
        return interrupt_status;
    case 0x4:
        return interrupt_mask;
    case 0x8: {
        u32 value = interrupt_control;
        interrupt_control = 0;
        UpdateInterrupts();
        return value;
    }
    default:
        log_fatal("InterruptController: handle read offset %02x", offset);
    }
}

void IOPInterruptController::WriteRegister(int offset, u32 data) {
    switch (offset) {
    case 0x0:
        log_debug("[IOP INTC] I_STAT write %08x", data);
        interrupt_status &= data & WRITE_MASK;
        UpdateInterrupts();
        break;
    case 0x4:
        log_debug("[IOP INTC] I_MASK write %08x", data);
        interrupt_mask = data & WRITE_MASK;
        UpdateInterrupts();
        break;
    case 0x8:
        interrupt_control = data & 0x1;
        UpdateInterrupts();
        break;
    default:
        log_fatal("InterruptController: handle write offset %02x", offset);
    }
}

void IOPInterruptController::RequestInterrupt(IOPInterruptSource source) {
    interrupt_status |= 1 << static_cast<u32>(source);
    UpdateInterrupts();
}

void IOPInterruptController::UpdateInterrupts() {
    cpu.SendInterruptSignal(interrupt_control && (interrupt_status & interrupt_mask));
}