#include "common/log.h"
#include "core/iop/sio2.h"

namespace iop {

SIO2::SIO2(INTC& intc) : intc(intc) {}

void SIO2::Reset() {
    control = 0;
    send1.fill(0);
    send2.fill(0);
    send3.fill(0);
}

u32 SIO2::ReadRegister(u32 addr) {
    switch (addr) {
    case 0x1f808264:
        // fifo out
        return 0;
    case 0x1f808268:
        return control;
    case 0x1f80826c:
        // response status 1
        return 0;
    case 0x1f808270:
        // response status 2
        return 0xf;
    case 0x1f808274:
        // response status 3
        return 0;
    case 0x1f808280:
        // istat
        return 0;
    default:
        common::Error("[iop::SIO2] handle read %08x", addr);
    }

    return 0;
}

void SIO2::WriteRegister(u32 addr, u32 value) {
    if (addr >= 0x1f808200 && addr < 0x1f808240) {
        int index = (addr - 0x1f808200) / 4;
        common::Log("[iop::SIO2] send3[%d] write %08x", index, value);
        send3[index] = value;
        return;
    }

    if (addr >= 0x1f808240 && addr < 0x1f808260) {
        int index = (addr - 0x1f808240) / 8;
        if (addr & 0x4) {
            common::Log("[iop::SIO2] send2[%d] write %08x", index, value);
            send2[index] = value;
        } else {
            common::Log("[iop::SIO2] send1[%d] write %08x", index, value);
            send1[index] = value;
        }
        return;
    }

    switch (addr) {
    case 0x1f808260:
        common::Log("[iop::SIO2] fifo write %08x", value);
        break;
    case 0x1f808268:
        control = value;

        if (value & 0x1) {
            intc.RequestInterrupt(InterruptSource::SIO2);
            control &= ~0x1;
        }

        if (value & 0x4) {
            common::Log("[iop::SIO2] reset sio2");
        }

        if (value & 0x8) {
            common::Log("[iop::SIO2] reset sio2");
        }

        break;
    case 0x1f808280:
        break;
    default:
        common::Error("[iop::SIO2] handle write %08x = %08x", addr, value);
    }
}

u8 SIO2::ReadDMA() {
    common::Log("[iop::SIO2] dma read");
    return 0;
}

void SIO2::WriteDMA(u8 data) {
    common::Log("[iop::SIO2] dma data %02x", data);
}

} // namespace iop