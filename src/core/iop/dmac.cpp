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
            case 9:
                DoSIF0Transfer();
                break;
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

    switch (index) {
    case 0x0:
        log_debug("[IOPDMAC %d] Dn_MADR read %08x", channel, channels[channel].address);
        return channels[channel].address;
    case 0x4:
        log_debug("[IOPDMAC %d] Dn_BCR read %08x", channel, (channels[channel].block_count << 16) | (channels[channel].block_size));
        return (channels[channel].block_count << 16) | (channels[channel].block_size);
    case 0x8:
        log_debug("[IOPDMAC %d] Dn_CHCR read %08x", channel, channels[channel].control);
        return channels[channel].control;
    case 0xC:
        log_debug("[IOPDMAC %d] Dn_TADR read %08x", channel, channels[channel].tag_address);
        return channels[channel].tag_address;
    default:
        log_fatal("[IOPDMAC] %08x", index);
    }
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

void IOPDMAC::DoSIF0Transfer() {
    Channel& channel = channels[10];

    if (channel.block_count) {
        // read data from iop ram and push to the sif0 fifo
        system->sif.WriteSIF0FIFO(system->iop_core->ReadWord(channel.address));

        channel.address += 4;
        channel.block_count--;
    } else if (channel.end_transfer) {
        log_fatal("handle end of transfer");
    } else {
        u32 data = system->iop_core->ReadWord(channel.tag_address);
        u32 block_count = system->iop_core->ReadWord(channel.tag_address + 4);

        log_debug("[IOPDMAC] read sif0 dmatag %016lx", ((u64)block_count << 32) | data);

        // round to the nearest 4
        channel.block_count = (block_count + 3) & 0xFFFFFFFC;
        channel.address = data & 0xFFFFFF;

        channel.tag_address += 8;

        // check channel control bit 8
        // for chain mode it means the dmatag is transferred before the data
        if (channel.control & (1 << 8)) {
            system->sif.WriteSIF0FIFO(system->iop_core->ReadWord(channel.tag_address));
            system->sif.WriteSIF0FIFO(system->iop_core->ReadWord(channel.tag_address + 4));
            channel.tag_address += 8;
        }

        if ((data & (1 << 30)) || (data & (1 << 31))) {
            channel.end_transfer = true;
        }
    }
}

void IOPDMAC::DoSIF1Transfer() {
    Channel& channel = channels[10];

    if (channel.block_count) {
        // transfer data from the sif1 fifo to iop ram
        if (system->sif.GetSIF1FIFOSize() > 0) {
            u32 data = system->sif.ReadSIF1FIFO();

            system->iop_core->WriteWord(channel.address, data);
            channel.address += 4;
            channel.block_count--;
        }
    } else if (channel.end_transfer) {
        EndTransfer(10);
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

void IOPDMAC::EndTransfer(int index) {
    channels[index].end_transfer = false;
    channels[index].control &= ~(1 << 24);

    // raise an interrupt in dicr2
    dicr2.flags |= (1 << (index - 7));

    CheckInterruptSignal();
}

void IOPDMAC::CheckInterruptSignal() {
    if (dicr2.flags & dicr2.masks) {
        system->iop_core->interrupt_controller.RequestInterrupt(IOPInterruptSource::DMA);
    }
}