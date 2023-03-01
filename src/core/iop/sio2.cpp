#include "common/log.h"
#include "core/iop/sio2.h"

namespace iop {

void SIO2::Reset() {
    control = 0;
}

u32 SIO2::ReadRegister(u32 addr) {
    switch (addr) {
    default:
        common::Error("[iop::SIO2] handle read %08x", addr);
    }

    return 0;
}

void SIO2::WriteRegister(u32 addr, u32 value) {
    switch (addr) {
    case 0x1f808268:
        if (value & 0x1) {
            common::Error("[iop::SIO2] start transfer");
        }

        if (value & 0x4) {
            common::Log("[iop::SIO2] reset sio2");
        }

        if (value & 0x8) {
            common::Log("[iop::SIO2] reset sio2");
        }

        control = value;
        break;
    default:
        common::Error("[iop::SIO2] handle write %08x = %08x", addr, value);
    }
}

} // namespace iop