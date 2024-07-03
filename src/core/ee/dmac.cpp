#include <cassert>
#include "common/log.h"
#include "common/memory.h"
#include "core/ee/dmac.h"
#include "core/system.h"

static const char* channel_names[10] = {
    "VIF0", "VIF1", "GIF", "IPU_FROM", "IPU_TO",
    "SIF0", "SIF1", "SIF2", "SPR_FROM", "SPR_TO",
};

namespace ee {

DMAC::DMAC(System& system) : system(system) {}

void DMAC::Reset() {
    control = 0;
    interrupt_status = 0;
    priority_control = 0;
    skip_quadword = 0;
    ringbuffer_size = 0;
    ringbuffer_offset = 0;
    disabled_status = 0x1201;

    for (int i = 0; i < 10; i++) {
        channels[i].control.data = 0;
        channels[i].address = 0;
        channels[i].tag_address = 0;
        channels[i].quadword_count = 0;
        channels[i].saved_tag_address0 = 0;
        channels[i].saved_tag_address1 = 0;
        channels[i].scratchpad_address = 0;
        channels[i].end_transfer = false;
    }
}

u32 DMAC::ReadChannel(u32 addr) {
    int index = GetChannelIndex(addr);

    switch (addr & 0xFF) {
    case 0x00:
        // common::Log("[DMAC %d] control read %08x", index, channels[index].control);
        return channels[index].control.data;
    case 0x10:
        return channels[index].address;
    case 0x20:
        return channels[index].quadword_count;
    case 0x30:
        return channels[index].tag_address;
    default:
        common::Error("[ee::DMAC] Handle %02x", addr & 0xFF);
    }

    return 0;
}

void DMAC::WriteRegister(u32 addr, u32 data) {
    switch (addr) {
    case 0x1000e000:
        common::Log("[ee::DMAC] D_CTRL write %08x", data);
        control = data;
        break;
    case 0x1000e010:
        common::Log("[ee::DMAC] D_STAT write %08x", data);

        // for bits (0..15) they get cleared if 1 is written
        interrupt_status &= ~(data & 0xffff);

        // for bits (16..31) they get reversed if 1 is written
        interrupt_status ^= (data & 0xffff0000);

        CheckInterruptSignal();
        break;
    case 0x1000e020:
        common::Log("[ee::DMAC] D_PCR write %08x", data);
        priority_control = data;
        break;
    case 0x1000e030:
        common::Log("[ee::DMAC] D_SQWC write %08x", data);
        skip_quadword = data;
        break;
    case 0x1000e040:
        common::Log("[ee::DMAC] D_RBSR write %08x", data);
        ringbuffer_size = data;
        break;
    case 0x1000e050:
        common::Log("[ee::DMAC] D_RBOR write %08x", data);
        ringbuffer_offset = data;
        break;
    case 0x1000f590:
        common::Log("[ee::DMAC] D_ENABLE write %08x", data);
        disabled_status = data;
        break;
    default:
        if (addr >= 0x1000e000) {
            common::Error("[ee::DMAC] handle write %08x = %08x", addr, data);
        } else {
            WriteChannel(addr, data);
        }

        break;
    }
}

void DMAC::WriteChannel(u32 addr, u32 data) {
    int index = GetChannelIndex(addr);
    const char* channel_name = channel_names[index];

    switch (addr & 0xff) {
    case 0x00:
        common::Log("[ee::DMAC] %s Dn_CHCR write %08x", channel_name, data);
        channels[index].control.data = data;

        StartTransfer(index);
        break;
    case 0x10:
        common::Log("[ee::DMAC] %s Dn_MADR write %08x", channel_name, data);
        channels[index].address = data & ~0xf;
        break;
    case 0x20:
        // In normal and interleaved mode, the transfer ends when QWC reaches zero. Chain mode behaves differently
        common::Log("[ee::DMAC] %s Dn_QWC write %08x", channel_name, data);
        channels[index].quadword_count = data & 0xffff;
        break;
    case 0x30:
        common::Log("[ee::DMAC] %s Dn_TADR write %08x", channel_name, data);
        channels[index].tag_address = data & ~0xf;
        break;
    case 0x40:
        common::Log("[ee::DMAC] %s Dn_ASR0 write %08x", channel_name, data);
        channels[index].saved_tag_address0 = data & ~0xf;
        break;
    case 0x50:
        common::Log("[ee::DMAC] %s Dn_ASR1 write %08x", channel_name, data);
        channels[index].saved_tag_address1 = data & ~0xf;
        break;
    case 0x80:
        common::Log("[ee::DMAC] %s Dn_SADR write %08x", channel_name, data);
        channels[index].scratchpad_address = data & ~0xf;
        break;
    default:
        common::Error("[ee::DMAC] Handle channel with identifier %02x and data %08x", addr & 0xff, data);
    }
}

int DMAC::GetChannelIndex(u32 addr) {
    auto channel = (addr >> 8) & 0xff;
    switch (channel) {
    case 0x80:
        return static_cast<int>(ChannelType::VIF0);
    case 0x90:
        return static_cast<int>(ChannelType::VIF1);
    case 0xA0:
        return static_cast<int>(ChannelType::GIF);
    case 0xB0:
        return static_cast<int>(ChannelType::IPUFrom);
    case 0xB4:
        return static_cast<int>(ChannelType::IPUTo);
    case 0xC0:
        return static_cast<int>(ChannelType::SIF0);
    case 0xC4:
        return static_cast<int>(ChannelType::SIF1);
    case 0xC8:
        return static_cast<int>(ChannelType::SIF2);
    case 0xD0:
        return static_cast<int>(ChannelType::FromSPR);
    case 0xD4:
        return static_cast<int>(ChannelType::ToSPR);
    default:
        common::Error("[ee::DMAC] Random behaviour!");
    }

    return 0;
}

u32 DMAC::ReadInterruptStatus() {
    common::Log("[ee::DMAC] D_STAT read %08x", interrupt_status);
    return interrupt_status;
}

u32 DMAC::ReadControl() {
    common::Log("[ee::DMAC] read control %08x", control);
    return control;
}

u32 DMAC::ReadPriorityControl() {
    common::Log("[ee::DMAC] read priority control %08x", priority_control);
    return priority_control;
}

u32 DMAC::ReadSkipQuadword() {
    common::Log("[ee::DMAC] read skip quadword %08x", skip_quadword);
    return skip_quadword;
}

void DMAC::CheckInterruptSignal() {
    bool irq = false;

    for (int i = 0; i < 10; i++) {
        if ((interrupt_status & (1 << i)) && (interrupt_status & (1 << (16 + i)))) {
            common::Log("[ee::DMAC] %s interrupt sent", channel_names[i]);
            irq = true;
            break;
        }
    }

    system.ee.RaiseInterrupt(1, irq);
}

// note:
// dmac can transfer one quadword (16 bytes / 128 bits) per bus cycle
void DMAC::Run(int cycles) {
    if (!(control & 0x1) || (disabled_status & (1 << 16))) {
        return;
    }

    while (cycles--) {
        for (int i = 0; i < 10; i++) {
            auto& channel = channels[i];
            if (channel.control.busy) {
                Transfer(i);
            }
        }
    }
}

void DMAC::Transfer(int index) {
    switch (static_cast<ChannelType>(index)) {
    case ChannelType::GIF:
        do_gif_transfer();
        break;
    case ChannelType::SIF0:
        do_sif0_transfer();
        break;
    case ChannelType::SIF1:
        do_sif1_transfer();
        break;
    case ChannelType::ToSPR:
        do_to_spr_transfer();
        break;
    default:
        common::Error("handle dma transfer with channel %d", index);
    }
}

void DMAC::do_gif_transfer() {
    auto& channel = channels[2];

    if (channel.quadword_count) {
        u128 data = system.ee.read<u128>(channel.address);

        system.gif.SendPath3(data);
        channel.address += 16;
        channel.quadword_count--;        
    } else {
        EndTransfer(2);
    }
}

void DMAC::do_sif0_transfer() {
    auto& channel = channels[5];

    if (channel.quadword_count) {
        // dmac can only transfer a quadword at a time
        if (system.sif.GetSIF0FIFOSize() >= 4) {
            for (int i = 0; i < 4; i++) {
                u32 data = system.sif.ReadSIF0FIFO();

                common::Log("[ee::DMAC] SIF0 reading data from fifo %08x", data);

                system.ee.write<u32>(channel.address, data);
                channel.address += 4;
            }

            channel.quadword_count--;
        }
    } else if (channel.end_transfer) {
        EndTransfer(5);
    } else {
        if (system.sif.GetSIF0FIFOSize() >= 2) {
            // form a dmatag
            u64 dma_tag = 0;

            dma_tag |= system.sif.ReadSIF0FIFO();
            dma_tag |= (u64)system.sif.ReadSIF0FIFO() << 32;

            common::Log("[ee::DMAC] SIF0 read DMATag %016lx", dma_tag);

            channel.quadword_count = dma_tag & 0xFFFF;
            channel.address = (dma_tag >> 32) & 0xFFFFFFF0;
            channel.tag_address += 16;

            // Update upper 16 bits of control with upper 16 bits of dma tag.
            channel.control.dmatag_upper = (dma_tag >> 16) & 0xffff;

            bool irq = (dma_tag >> 31) & 0x1;
            if (irq && channel.control.dmatag_irq) {
                channel.end_transfer = true;
            }
        }
    }
}

void DMAC::do_sif1_transfer() {
    auto& channel = channels[6];
    if (channel.quadword_count) {
        // push data to the sif1 fifo
        u128 data = system.ee.read<u128>(channel.address);

        common::Log("[ee::DMAC] SIF1 Fifo write %016lx%016lx dstat %08x", data.hi, data.lo, interrupt_status);

        system.sif.write_sif1_fifo(data);

        // madr and qwc must be updated as the transfer proceeds
        channel.address += 16;
        channel.quadword_count--;
    } else if (channel.end_transfer) {
        EndTransfer(6);
    } else {
        DoSourceChain(6);
    }
}

void DMAC::do_to_spr_transfer() {
    // This channel will transfer in a burst.
    // It can also use source chain and interleaving.
    // TODO: do we need to handle cycle stealing?

    // Scratchpad is considered a peripheral, so if chain mode is enabled
    // it's source chain mode.
    auto& channel = channels[9];

    assert(channel.control.mode == Channel::Mode::Normal);
    assert(!channel.control.from_memory);

    if (channel.control.mode != Channel::Mode::Normal) {
        LOG_TODO_NO_ARGS("handle non-normal mode for to spr transfer");
    }

    if (channel.quadword_count > 0) {
        for (u32 i = 0; i < channel.quadword_count; i++) {
            // Ensure the transfer address is in the physical address range.
            u128 data = read_u128(channel.address & 0x7fffffff);

            // Select writes to scratchpad.
            write_u128(channel.scratchpad_address | (1 << 31), data);

            // Update channel registers
            channel.address += 16;
            channel.scratchpad_address += 16;
            channel.quadword_count--;
        }

        EndTransfer(9);
    } else {
        LOG_TODO_NO_ARGS("handle to spr transfer with no quadword count");
    }
}

void DMAC::StartTransfer(int index) {
    common::Log("[ee::DMAC] %s start transfer", channel_names[index]);

    // in normal mode we shouldn't worry about dmatag reading
    channels[index].end_transfer = channels[index].control.mode == Channel::Mode::Normal;
}

void DMAC::EndTransfer(int index) {
    common::Log("[ee::DMAC] %s end transfer", channel_names[index]);

    channels[index].end_transfer = false;
    channels[index].control.busy = false;

    // raise the stat flag in the stat register
    interrupt_status |= (1 << index);

    CheckInterruptSignal();
}

void DMAC::DoSourceChain(int index) {
    auto& channel = channels[index];
    u128 data = system.ee.read<u128>(channel.tag_address);

    // TODO: create a union type for dma tag to easily extract fields
    u64 dma_tag = data.lo;

    common::Log("[ee::DMAC] %s read DMATag %016lx d stat %08x", channel_names[index], dma_tag, interrupt_status);

    channel.quadword_count = dma_tag & 0xFFFF;

    // Update upper 16 bits of control with upper 16 bits of dma tag.
    channel.control.dmatag_upper = (dma_tag >> 16) & 0xffff;
    
    u8 id = (dma_tag >> 28) & 0x7;

    // lower 4 bits must be 0
    u32 addr = (dma_tag >> 32) & 0xFFFFFFF0;

    switch (id) {
    case 0:
        // MADR=DMAtag.ADDR
        // TADR+=16
        // tag_end=true
        channel.address = addr;
        channel.tag_address += 16;
        channel.end_transfer = true;
        break;
    case 2:
        // MADR=TADR+16
        // TADR=DMAtag.ADDR
        channel.address = channel.tag_address + 16;
        channel.tag_address = addr;
        break;
    case 3:
        // MADR=DMAtag.ADDR
        // TADR+=16
        channel.address = addr;
        channel.tag_address += 16;
        break;
    default:
        common::Error("[ee::DMAC] %s handle DMATag id %d", channel_names[index], id);
    }

    bool irq = (dma_tag >> 31) & 0x1;
    if (irq && channel.control.dmatag_irq) {
        channel.end_transfer = true;
    }
}

u128 DMAC::read_u128(u32 addr) {
    if (addr < 0x2000000) {
        return common::Read<u128>(system.ee.rdram(), addr);
    }

    LOG_TODO("handle dma 128-bit read with address %08x", addr);
}

void DMAC::write_u128(u32 addr, u128 data) {
    if ((addr & (1 << 31)) || (addr & 0x07000000) == 0x07000000) {
        u32 masked_addr = addr & 0x3ff0;
        common::Write<u128>(system.ee.scratchpad(), data, masked_addr);
        return;
    }

    LOG_TODO("handle dma 128-bit write with address %08x", addr);
}

} // namespace ee