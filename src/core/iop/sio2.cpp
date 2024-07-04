#include "common/log.h"
#include "core/iop/sio2.h"

namespace iop {

SIO2::SIO2(INTC& intc) : intc(intc) {}

void SIO2::Reset() {
    control = 0;
    send1.fill(0);
    send2.fill(0);
    send3.fill(0);
    fifo.Reset();
}

u32 SIO2::ReadRegister(u32 addr) {
    switch (addr) {
    case 0x1f808264:
        // fifo out
        LOG_TODO("[iop::SIO2] fifo read %08x", 0);
        return 0;
    case 0x1f808268:
        common::Log("[iop::SIO2] control read %08x", control);
        return control;
    case 0x1f80826c:
        // response status 1
        LOG_TODO("[iop::SIO2] recv1 read %08x", 0x1d100);
        return 0x1d100;
    case 0x1f808270:
        // response status 2
        common::Log("[iop::SIO2] recv2 read %08x", 0xf);
        return 0xf;
    case 0x1f808274:
        // response status 3
        LOG_TODO("[iop::SIO2] recv3 read %08x", 0);
        return 0;
    case 0x1f808280:
        // istat
        return 0;
    default:
        LOG_TODO("[iop::SIO2] handle read %08x", addr);
    }

    return 0;
}

void SIO2::WriteRegister(u32 addr, u32 value) {
    if (addr >= 0x1f808200 && addr < 0x1f808240) {
        int index = (addr - 0x1f808200) / 4;
        LOG_TODO("[iop::SIO2] send3[%d] write %08x", index, value);
        send3[index] = value;
        return;
    }

    if (addr >= 0x1f808240 && addr < 0x1f808260) {
        int index = (addr - 0x1f808240) / 8;
        if (addr & 0x4) {
            LOG_TODO("[iop::SIO2] send2[%d] write %08x", index, value);
            send2[index] = value;
        } else {
            LOG_TODO("[iop::SIO2] send1[%d] write %08x", index, value);
            send1[index] = value;
        }
        return;
    }

    switch (addr) {
    case 0x1f808260:
        LOG_TODO("[iop::SIO2] fifo write %08x", value);
        break;
    case 0x1f808268:
        control = value;

        if (value & 0x1) {
            LOG_TODO_NO_ARGS("[iop::SIO2] raise sio2 interrupt");
            intc.RequestInterrupt(InterruptSource::SIO2);
            control &= ~0x1;
        }

        if (value & 0xc) {
            // TODO: reset state here for next transfer
            common::Log("[iop::SIO2] reset sio2");
        }

        break;
    case 0x1f808280:
        break;
    default:
        LOG_TODO("[iop::SIO2] handle write %08x = %08x", addr, value);
    }
}

u8 SIO2::ReadDMA() {
    LOG_TODO_NO_ARGS("[iop::SIO2] dma read");
    return 0;
}

void SIO2::WriteDMA(u8 data) {
    LOG_TODO("[iop::SIO2] dma data %02x", data);
}

} // namespace iop