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
        interrupt_status &= data & WRITE_MASK;
        break;
    case 0x4:
        interrupt_mask = data & WRITE_MASK;
        break;
    case 0x8:
        interrupt_control = data & 0x1;
        break;
    default:
        log_fatal("InterruptController: handle write offset %02x", offset);
    }
}

void IOPInterruptController::RequestInterrupt(InterruptSource source) {
    interrupt_status |= 1 << static_cast<u32>(source);
}

void IOPInterruptController::UpdateInterrupts() {
    if (interrupt_control && (interrupt_status & interrupt_mask)) {
        log_fatal("handle enable");
        // cpu.cpu_cpu.EnableHardwareInterrupt();
    } else {
        cpu.DisableHardwareInterrupt();
    }
}