#include "common/log.h"
#include "core/iop/dmac.h"

void IOPDMAC::Reset() {
    for (int i = 0; i < 13; i++) {
        channels[i].address = 0;
        channels[i].block_size = 0;
        channels[i].block_count = 0;
        channels[i].control = 0;
        channels[i].tag_address = 0;
    }

    dpcr = 0x07777777;
    dpcr2 = 0x07777777;
    dicr = 0;
    dicr2 = 0;
    global_dma_enable = false;
    global_dma_interrupt_control = false;
}

u32 IOPDMAC::ReadRegister(u32 addr) {
    switch (addr) {
    case 0x1F801578:
        return global_dma_enable;
    default:
        log_fatal("handle %08x", addr);
    }
}

void IOPDMAC::WriteRegister(u32 addr, u32 data) {
    switch (addr) {
    case 0x1F8010F0:
        dpcr = data;
        break;
    case 0x1F8010F4:
        dicr = data;
        break;
    case 0x1F801570:
        dpcr2 = data;
        break;
    case 0x1F801574:
        dicr2 = data;
        break;
    case 0x1F801578:
        global_dma_enable = data & 0x1;
        break;
    case 0x1F80157C:
        global_dma_interrupt_control = data & 0x1;
        break;
    default:
        WriteChannel(addr, data);
        break;
    }
}

int IOPDMAC::GetChannelIndex(u32 addr) {
    int channel = (addr >> 4) & 0xF;

    // this allows us to map the 2nd nibble to channel index
    if (channel >= 8) {
        return channel - 8;
    } else {
        return channel + 7;
    }
}

void IOPDMAC::WriteChannel(u32 addr, u32 data) {
    int channel = GetChannelIndex(addr);
    int index = addr & 0xF;

    switch (index) {
    case 0x0:
        channels[channel].address = data & 0xFFFFFF;
        break;
    case 0x4:
        channels[channel].block_size = data & 0xFFFF;
        channels[channel].block_count = (data >> 16) & 0xFFFF;
        break;
    case 0x8:
        channels[channel].control = data;
        break;
    case 0xC:
        channels[channel].tag_address = data;
        break;
    default:
        log_fatal("handle %02x", index);
    }
}