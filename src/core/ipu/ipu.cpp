#include <core/ipu/ipu.h>

void IPU::Reset() {
    control = 0;
    command = 0;
}

void IPU::SystemReset() {
    log_warn("[IPU] system reset");
}

void IPU::WriteControl(u32 data) {
    if (data & 0x40000000) {
        SystemReset();
    }

    if (data != 0x40000000) {
        log_fatal("handle");
    }

    log_warn("[IPU] write control %08x", data);
    control = data;
}

u32 IPU::ReadControl() {
    return control;
}

void IPU::WriteCommand(u32 data) {
    log_warn("[IPU] write command %08x", data);

    command = data;
}