#include <core/iop/cpu_core.h>
#include <core/system.h>

IOPCore::IOPCore(System* system) : system(system), interrupt_controller(*this) {

}

void IOPCore::EnableHardwareInterrupt() {
    log_fatal("handle")
}

void IOPCore::DisableHardwareInterrupt() {
    cop0.gpr[13] &= ~(1 << 10);
}