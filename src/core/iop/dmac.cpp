#include <assert.h>
#include "common/log.h"
#include "core/iop/dmac.h"
#include "core/system.h"

IOPDMAC::IOPDMAC(System* system) : system(system) {}

void IOPDMAC::Reset() {
    for (int i = 0; i < 13; i++) {
        channels[i].address = 0;
        channels[i].block_size = 0;
        channels[i].block_count = 0;
        channels[i].control = 0;
        channels[i].tag_address = 0;
        channels[i].end_transfer = false;
    }

    dpcr = 0x07777777;
    dpcr2 = 0x07777777;
    dicr.data = 0;
    dicr2.data = 0;
    global_dma_enable = false;
    global_dma_interrupt_control = false;
}

void IOPDMAC::Run(int cycles) {
    for (int i = 7; i < 13; i++) {
        if (GetChannelEnable(i) && (channels[i].control & (1 << 24))) {
            switch (i) {
            case 10:
                DoSIF1Transfer();
                break;
            default:
                log_fatal("[IOPDMAC] handle transfer for channel %d", i);
            }
        }
    }
}

u32 IOPDMAC::ReadRegister(u32 addr) {
    switch (addr) {
    case 0x1F8010F0:
        return dpcr;
    case 0x1F8010F4:
        log_debug("[IOPDMAC] dicr read %08x", dicr.data);
        return dicr.data;
    case 0x1F801570:
        return dpcr2;
    case 0x1F801574:
        log_debug("[IOPDMAC] dicr2 read %08x", dicr2.data);
        return dicr2.data;
    case 0x1F801578:
        return global_dma_enable;
    case 0x1F80157C:
        return global_dma_interrupt_control;
    default:
        return ReadChannel(addr);
    }
}

u32 IOPDMAC::ReadChannel(u32 addr) {
    int channel = GetChannelIndex(addr);
    int index = addr & 0xF;

    log_fatal("handle");
}

void IOPDMAC::WriteRegister(u32 addr, u32 data) {
    switch (addr) {
    case 0x1F8010F0:
        log_debug("[IOPDMAC] dpcr write %08x", data);
        dpcr = data;
        break;
    case 0x1F8010F4:
        log_debug("[IOPDMAC] dicr write %08x", data);
        dicr.data = data;

        // writing 1 to the flag bits clears them
        dicr.flags &= ~((data >> 24) & 0x7F);

        // update dicr master flag
        dicr.master_interrupt_flag = dicr.force_irq || (dicr.master_interrupt_enable && (dicr.masks & dicr.flags));
        break;
    case 0x1F801570:
        dpcr2 = data;
        break;
    case 0x1F801574:
        log_debug("[IOPDMAC] dicr2 write %08x", data);
        dicr2.data = data;

        // writing 1 to the flag bits clears them
        dicr2.flags &= ~((data >> 24) & 0x7F);
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
        channel -= 8;
    } else {
        channel += 7;
    }

    return channel;
}

bool IOPDMAC::GetChannelEnable(int index) {
    return (dpcr2 >> (3 + ((index - 7) * 4))) & 0x1;
}

void IOPDMAC::WriteChannel(u32 addr, u32 data) {
    int channel = GetChannelIndex(addr);
    int index = addr & 0xF;

    switch (index) {
    case 0x0:
        log_warn("[IOPDMAC %d] address write %08x", channel, data);
        channels[channel].address = data & 0xFFFFFF;
        break;
    case 0x4:
        log_warn("[IOPDMAC %d] block size and count write %08x", channel, data);
        channels[channel].block_size = data & 0xFFFF;
        channels[channel].block_count = (data >> 16) & 0xFFFF;
        break;
    case 0x8:
        log_warn("[IOPDMAC %d] control write %08x", channel, data);
        channels[channel].control = data;
        break;
    case 0xC:
        log_warn("[IOPDMAC %d] tag address %08x", channel, data);
        channels[channel].tag_address = data;
        break;
    default:
        log_fatal("handle %02x", index);
    }
}

void IOPDMAC::DoSIF1Transfer() {
    Channel& channel = channels[10];

    // log_debug("do sif1 transfer");

    if (channel.block_count) {
        log_fatal("handle non zero sif1 transfer block count");
    } else if (channel.end_transfer) {
        log_fatal("handle end transfer");
    } else {
        if (system->sif.GetSIF1FIFOSize() >= 4) {
            u32 data = system->sif.ReadSIF1FIFO();

            channel.address = data & 0xFFFFFF;
            channel.block_count = system->sif.ReadSIF1FIFO();

            // since the ee would've pushed quads one at a time we need to remove the upper 2 words
            system->sif.ReadSIF1FIFO();
            system->sif.ReadSIF1FIFO();

            if ((data & (1 << 30)) || (data & (1 << 31))) {
                channel.end_transfer = true;
            }
        }
    }
}