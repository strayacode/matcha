#include "common/log.h"
#include "core/iop/cdvd.h"

namespace iop {

void CDVD::Reset() {
    // set bit 6 to 1 (drive is ready)
    n_command_status = 0x40;

    // set bit 6 to 1 (no data available)
    s_command_status = 0x40;

    s_command = 0;
}

u32 CDVD::ReadRegister(u32 addr) {
    switch (addr) {
    case 0x1f402005:
        return n_command_status;
    case 0x1f402016:
        return s_command;
    case 0x1f402017:
        return s_command_status;
    default:
        common::Error("[iop::CDVD] handle read %08x", addr);
    }

    return 0;
}

void CDVD::WriteRegister(u32 addr, u32 value) {
    switch (addr) {
    case 0x1f402016:
        s_command = value;
        DoSCommand();
        break;
    default:
        common::Error("[iop::CDVD] handle write %08x = %08x", addr, value);
    }
}

void CDVD::DoSCommand() {
    // TODO: handle s command results
    switch (s_command) {
    case 0x15:
        common::Log("[iop::CDVD] ForbidDVD");
        break;
    default:
        common::Error("[iop::CDVD] handle s command %02x", s_command);
    }
}

} // namespace iop