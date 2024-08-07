#include "common/log.h"
#include "common/log.h"
#include "core/iop/dmac.h"
#include "core/system.h"

namespace iop {

DMAC::DMAC(System& system, SIO2& sio2) : system(system), sio2(sio2) {}

void DMAC::Reset() {
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

void DMAC::Run(int cycles) {
    for (int i = 7; i < 13; i++) {
        if (GetChannelEnable(i) && (channels[i].control & (1 << 24))) {
            switch (i) {
            case 7:
                DoSPU2Transfer();
                break;
            case 9:
                DoSIF0Transfer();
                break;
            case 10:
                DoSIF1Transfer();
                break;
            case 11:
                DoSIO2InTransfer();
                break;
            case 12:
                DoSIO2OutTransfer();
                break;
            default:
                common::Error("[iop::DMAC] handle transfer for channel %d", i);
            }
        }
    }
}

u32 DMAC::ReadRegister(u32 addr) {
    switch (addr) {
    case 0x1F8010F0:
        return dpcr;
    case 0x1F8010F4:
        common::Log("[iop::DMAC] dicr read %08x", dicr.data);
        return dicr.data;
    case 0x1F801570:
        return dpcr2;
    case 0x1F801574:
        common::Log("[iop::DMAC] dicr2 read %08x", dicr2.data);
        return dicr2.data;
    case 0x1F801578:
        return global_dma_enable;
    case 0x1F80157C:
        return global_dma_interrupt_control;
    default:
        return ReadChannel(addr);
    }
}

u32 DMAC::ReadChannel(u32 addr) {
    int channel = GetChannelIndex(addr);
    int index = addr & 0xF;

    switch (index) {
    case 0x0:
        common::Log("[iop::DMAC %d] Dn_MADR read %08x", channel, channels[channel].address);
        return channels[channel].address;
    case 0x4:
        common::Log("[iop::DMAC %d] Dn_BCR read %08x", channel, (channels[channel].block_count << 16) | (channels[channel].block_size));
        return (channels[channel].block_count << 16) | (channels[channel].block_size);
    case 0x8:
        common::Log("[iop::DMAC %d] Dn_CHCR read %08x", channel, channels[channel].control);
        return channels[channel].control;
    case 0xC:
        common::Log("[iop::DMAC %d] Dn_TADR read %08x", channel, channels[channel].tag_address);
        return channels[channel].tag_address;
    default:
        common::Error("[iop::DMAC] %08x", index);
    }

    return 0;
}

void DMAC::WriteRegister(u32 addr, u32 data) {
    switch (addr) {
    case 0x1F8010F0:
        common::Log("[iop::DMAC] dpcr write %08x", data);
        dpcr = data;
        break;
    case 0x1f8010f4: {
        common::Log("[iop::DMAC] dicr write %08x", data);
        u8 flags = dicr.flags;
        dicr.data = data;

        // writing 1 to the flag bits clears them
        dicr.flags = flags & ~((data >> 24) & 0x7f);

        // update dicr master flag
        dicr.master_interrupt_flag = dicr.force_irq || (dicr.master_interrupt_enable && (dicr.masks & dicr.flags));

        if (dicr.force_irq) {
            common::Error("force irq dicr");
        }
        break;
    }
    case 0x1f801570:
        dpcr2 = data;
        break;
    case 0x1f801574: {
        common::Log("[iop::DMAC] dicr2 write %08x", data);
        u8 flags = dicr2.flags;
        dicr2.data = data;

        // writing 1 to the flag bits clears them
        dicr2.flags = flags & ~((data >> 24) & 0x7f);

        if (dicr2.force_irq) {
            common::Error("force irq dicr2");
        }
        break;
    }
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

int DMAC::GetChannelIndex(u32 addr) {
    int channel = (addr >> 4) & 0xF;

    // this allows us to map the 2nd nibble to channel index
    if (channel >= 8) {
        channel -= 8;
    } else {
        channel += 7;
    }

    return channel;
}

bool DMAC::GetChannelEnable(int index) {
    return (dpcr2 >> (3 + ((index - 7) * 4))) & 0x1;
}

void DMAC::WriteChannel(u32 addr, u32 data) {
    int channel = GetChannelIndex(addr);
    int index = addr & 0xF;

    switch (index) {
    case 0x0:
        common::Log("[iop::DMAC %d] address write %08x", channel, data);
        channels[channel].address = data & 0xFFFFFF;
        break;
    case 0x4:
        common::Log("[iop::DMAC %d] block size and count write %08x", channel, data);
        channels[channel].block_size = data & 0xFFFF;
        channels[channel].block_count = (data >> 16) & 0xFFFF;
        break;
    case 0x6:
        common::Log("[iop::DMAC %d] block count write %08x", channel, data);
        channels[channel].block_count = data & 0xFFFF;
        break;
    case 0x8:
        common::Log("[iop::DMAC %d] control write %08x", channel, data);
        channels[channel].control = data;

        if (data & (1 << 24)) {
            common::Log("[iop::DMAC %d] transfer started", channel);
        }

        break;
    case 0xC:
        common::Log("[iop::DMAC %d] tag address %08x", channel, data);
        channels[channel].tag_address = data;
        break;
    default:
        common::Error("handle %02x", index);
    }
}

void DMAC::DoSIF0Transfer() {
    Channel& channel = channels[9];

    if (channel.block_count) {
        // read data from iop ram and push to the sif0 fifo
        system.sif.write_sif0_fifo(system.iop.Read<u32>(channel.address));

        channel.address += 4;
        channel.block_count--;
    } else if (channel.end_transfer) {
        EndTransfer(9);
    } else {
        u32 data = system.iop.Read<u32>(channel.tag_address);
        u32 block_count = system.iop.Read<u32>(channel.tag_address + 4);

        common::Log("[iop::DMAC] SIF0 read DMATag %016lx", ((u64)block_count << 32) | data);

        system.sif.write_sif0_fifo(system.iop.Read<u32>(channel.tag_address + 8));
        system.sif.write_sif0_fifo(system.iop.Read<u32>(channel.tag_address + 12));

        // common::Log("[iop::DMAC] read sif0 dmatag %016lx", ((u64)block_count << 32) | data);

        // round to the nearest 4
        channel.block_count = (block_count + 3) & 0xFFFFFFFC;
        channel.address = data & 0xFFFFFF;

        channel.tag_address += 16;

        bool irq = (data >> 30) & 0x1;
        bool end_transfer = (data >> 31) & 0x1;

        if (irq || end_transfer) {
            channel.end_transfer = true;
        }
    }
}

void DMAC::DoSIF1Transfer() {
    Channel& channel = channels[10];

    if (channel.block_count) {
        // transfer data from the sif1 fifo to iop ram
        if (system.sif.GetSIF1FIFOSize() > 0) {
            u32 data = system.sif.ReadSIF1FIFO();

            system.iop.Write<u32>(channel.address, data);
            channel.address += 4;
            channel.block_count--;
        }
    } else if (channel.end_transfer) {
        EndTransfer(10);
    } else {
        if (system.sif.GetSIF1FIFOSize() >= 4) {
            u64 dma_tag = 0;

            dma_tag |= system.sif.ReadSIF1FIFO();
            dma_tag |= (u64)system.sif.ReadSIF1FIFO() << 32;

            common::Log("[iop::DMAC] SIF1 read DMATag %016lx", dma_tag);

            channel.address = dma_tag & 0xFFFFFF;
            channel.block_count = dma_tag >> 32;

            // since the ee would've pushed quads one at a time we need to remove the upper 2 words
            system.sif.ReadSIF1FIFO();
            system.sif.ReadSIF1FIFO();

            bool irq = (dma_tag >> 30) & 0x1;
            bool end_transfer = (dma_tag >> 31) & 0x1;

            if (irq || end_transfer) {
                channel.end_transfer = true;
            }
        }
    }
}

// TODO: handle spu2 chain mode
void DMAC::DoSPU2Transfer() {
    Channel& channel = channels[7];

    if (channel.block_count) {
        // ignore the spu2 for now
        channel.block_count--;
    } else {
        EndTransfer(7);
    }
}

void DMAC::DoSIO2InTransfer() {
    Channel& channel = channels[11];
    int length = 4;

    for (int i = 0; i < length; i++) {
        u8 data = system.iop.Read<u8>(channel.address);
        sio2.WriteDMA(data);
        channel.address++;
    }
    
    channel.block_count--;
    if (channel.block_count == 0) {
        EndTransfer(11);
    }

    common::Log("[iop::DMAC sio2in] end transfer flags %08x masks %08x", dicr2.flags, dicr2.masks);
}

void DMAC::DoSIO2OutTransfer() {
    Channel& channel = channels[12];
    int length = 4;

    for (int i = 0; i < length; i++) {
        u8 data = sio2.ReadDMA();
        system.iop.Write<u8>(channel.address, data);
        channel.address++;
    }

    channel.block_count--;
    if (channel.block_count == 0) {
        EndTransfer(12);
    }

    common::Log("[iop::DMAC sio2out] end transfer flags %08x masks %08x", dicr2.flags, dicr2.masks);
}

void DMAC::EndTransfer(int index) {
    common::Log("[iop::DMAC %d] end transfer", index);

    if (index < 7) {
        common::Error("[iop::DMAC] handle index < 7, %d", index);
    }

    // hack for now for spu2 status register to be updated
    if (index == 7) {
        system.spu2.RequestInterrupt();
    }

    channels[index].end_transfer = false;
    channels[index].control &= ~(1 << 24);

    // raise an interrupt in dicr2
    dicr2.flags |= (1 << (index - 7));

    if (dicr2.flags & dicr2.masks) {
        common::Log("[iop::DMAC %d] interrupt was requested", index);
        system.iop.intc.RequestInterrupt(InterruptSource::DMA);
    }
}

} // namespace iop