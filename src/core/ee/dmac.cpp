#include "common/log.h"
#include "core/ee/dmac.h"
#include "core/system.h"

static const char* channel_names[10] = {
    "VIF0", "VIF1", "GIF", "IPU_FROM", "IPU_TO",
    "SIF0", "SIF1", "SIF2", "SPR_FROM", "SPR_TO",
};

DMAC::DMAC(System& system) : system(system) {}

void DMAC::Reset() {
    for (int i = 0; i < 10; i++) {
        channels[i].control = 0;
        channels[i].address = 0;
        channels[i].tag_address = 0;
        channels[i].quadword_count = 0;
        channels[i].saved_tag_address0 = 0;
        channels[i].saved_tag_address1 = 0;
        channels[i].scratchpad_address = 0;
        channels[i].end_transfer = false;
    }

    control = 0;
    interrupt_status = 0;
    priority_control = 0;
    skip_quadword = 0;
    ringbuffer_size = 0;
    ringbuffer_offset = 0;
    disabled_status = 0x1201;
}

u32 DMAC::ReadChannel(u32 addr) {
    int index = GetChannelIndex(addr);

    switch (addr & 0xFF) {
    case 0x00:
        // common::Debug("[DMAC %d] control read %08x", index, channels[index].control);
        return channels[index].control;
    case 0x10:
        return channels[index].address;
    case 0x20:
        return channels[index].quadword_count;
    case 0x30:
        return channels[index].tag_address;
    default:
        common::Error("[DMAC] Handle %02x", addr & 0xFF);
    }

    return 0;
}

void DMAC::WriteRegister(u32 addr, u32 data) {
    switch (addr) {
    case 0x1000E000:
        common::Debug("[DMAC] D_CTRL write %08x", data);
        control = data;
        break;
    case 0x1000E010:
        common::Log("[DMAC] D_STAT write %08x", data);

        // for bits (0..15) they get cleared if 1 is written
        interrupt_status &= ~(data & 0xFFFF);

        // for bits (16..31) they get reversed if 1 is written
        interrupt_status ^= (data & 0xFFFF0000);

        CheckInterruptSignal();
        break;
    case 0x1000E020:
        common::Debug("[DMAC] D_PCR write %08x", data);
        priority_control = data;
        break;
    case 0x1000E030:
        common::Debug("[DMAC] D_SQWC write %08x", data);
        skip_quadword = data;
        break;
    case 0x1000E040:
        common::Debug("[DMAC] D_RBSR write %08x", data);
        ringbuffer_size = data;
        break;
    case 0x1000E050:
        common::Debug("[DMAC] D_RBOR write %08x", data);
        ringbuffer_offset = data;
        break;
    case 0x1000F590:
        common::Debug("[DMAC] D_ENABLE write %08x", data);
        disabled_status = data;
        break;
    default:
        if (addr >= 0x1000E000) {
            common::Error("[DMAC] handle write %08x = %08x", addr, data);
        } else {
            WriteChannel(addr, data);
        }

        break;
    }
}

void DMAC::WriteChannel(u32 addr, u32 data) {
    int index = GetChannelIndex(addr);
    const char* channel_name = channel_names[index];

    switch (addr & 0xFF) {
    case 0x00:
        common::Debug("[DMAC] %s Dn_CHCR write %08x", channel_name, data);
        channels[index].control = data;

        StartTransfer(index);
        break;
    case 0x10:
        common::Debug("[DMAC] %s Dn_MADR write %08x", channel_name, data);
        channels[index].address = data & ~0xF;
        break;
    case 0x20:
        // In normal and interleaved mode, the transfer ends when QWC reaches zero. Chain mode behaves differently
        common::Debug("[DMAC] %s Dn_QWC write %08x", channel_name, data);
        channels[index].quadword_count = data & 0xFFFF;
        break;
    case 0x30:
        common::Debug("[DMAC] %s Dn_TADR write %08x", channel_name, data);
        channels[index].tag_address = data & ~0xF;
        break;
    case 0x40:
        common::Debug("[DMAC] %s Dn_ASR0 write %08x", channel_name, data);
        channels[index].saved_tag_address0 = data & ~0xF;
        break;
    case 0x50:
        common::Debug("[DMAC] %s Dn_ASR1 write %08x", channel_name, data);
        channels[index].saved_tag_address1 = data & ~0xF;
        break;
    case 0x80:
        common::Debug("[DMAC] %s Dn_SADR write %08x", channel_name, data);
        channels[index].scratchpad_address = data & ~0xF;
        break;
    default:
        common::Error("[DMAC] Handle channel with identifier %02x and data %08x", addr & 0xFF, data);
    }
}

int DMAC::GetChannelIndex(u32 addr) {
    u8 channel = (addr >> 8) & 0xff;

    switch (channel) {
    case 0x80:
        return static_cast<int>(DMAChannelType::VIF0);
        break;
    case 0x90:
        return static_cast<int>(DMAChannelType::VIF1);
        break;
    case 0xA0:
        return static_cast<int>(DMAChannelType::GIF);
        break;
    case 0xB0:
        return static_cast<int>(DMAChannelType::IPUFrom);
        break;
    case 0xB4:
        return static_cast<int>(DMAChannelType::IPUTo);
        break;
    case 0xC0:
        return static_cast<int>(DMAChannelType::SIF0);
        break;
    case 0xC4:
        return static_cast<int>(DMAChannelType::SIF1);
        break;
    case 0xC8:
        return static_cast<int>(DMAChannelType::SIF2);
        break;
    case 0xD0:
        return static_cast<int>(DMAChannelType::SPRFrom);
        break;
    case 0xD4:
        return static_cast<int>(DMAChannelType::SPRTo);
        break;
    default:
        common::Error("[DMAC] Random behaviour!");
    }

    return 0;
}

u32 DMAC::ReadInterruptStatus() {
    common::Log("[DMAC] D_STAT read %08x", interrupt_status);
    return interrupt_status;
}

u32 DMAC::ReadControl() {
    common::Debug("[DMAC] read control %08x", control);
    return control;
}

u32 DMAC::ReadPriorityControl() {
    common::Warn("[DMAC] read priority control %08x", priority_control);
    return priority_control;
}

u32 DMAC::ReadSkipQuadword() {
    common::Warn("[DMAC] read skip quadword %08x", skip_quadword);
    return skip_quadword;
}

void DMAC::CheckInterruptSignal() {
    bool irq = false;

    for (int i = 0; i < 10; i++) {
        if ((interrupt_status & (1 << i)) && (interrupt_status & (1 << (16 + i)))) {
            common::Log("[DMAC] %s interrupt sent", channel_names[i]);
            irq = true;
            break;
        }
    }

    system.ee.RaiseInterrupt(1, irq);
}

// note:
// dmac can transfer one quadword (16 bytes / 128 bits) per bus cycle
void DMAC::Run(int cycles) {
    // don't run anything if the dmac is not enabled
    if (((control & 0x1) == false) || (disabled_status & (1 << 16))) {
        return;
    }

    while (cycles--) {
        // run each channel
        for (int i = 0; i < 10; i++) {
            DMAChannel& channel = channels[i];

            if (channel.control & (1 << 8)) {
                Transfer(i);
            }
        }
    }
}

void DMAC::Transfer(int index) {
    switch (static_cast<DMAChannelType>(index)) {
    case DMAChannelType::GIF:
        DoGIFTransfer();
        break;
    case DMAChannelType::SIF0:
        DoSIF0Transfer();
        break;
    case DMAChannelType::SIF1:
        DoSIF1Transfer();
        break;
    default:
        common::Error("handle %d", index);
    }
}

void DMAC::DoGIFTransfer() {
    DMAChannel& channel = channels[2];

    if (channel.quadword_count) {
        u128 data = system.ee.Read<u128>(channel.address);

        system.gif.SendPath3(data);
        channel.address += 16;
        channel.quadword_count--;        
    } else {
        EndTransfer(2);
    }
}

void DMAC::DoSIF0Transfer() {
    DMAChannel& channel = channels[5];

    if (channel.quadword_count) {
        // dmac can only transfer a quadword at a time
        if (system.sif.GetSIF0FIFOSize() >= 4) {
            for (int i = 0; i < 4; i++) {
                u32 data = system.sif.ReadSIF0FIFO();

                common::Log("[DMAC] SIF0 reading data from fifo %08x", data);

                system.ee.Write<u32>(channel.address, data);
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

            common::Log("[DMAC] SIF0 read DMATag %016lx", dma_tag);

            channel.quadword_count = dma_tag & 0xFFFF;
            channel.address = (dma_tag >> 32) & 0xFFFFFFF0;
            channel.tag_address += 16;
            channel.control = (channel.control & 0xFFFF) | (dma_tag & 0xFFFF0000);

            bool irq = (dma_tag >> 31) & 0x1;
            bool tie = (channel.control >> 7) & 0x1;

            if (irq && tie) {
                channel.end_transfer = true;
            }
        }
    }
}

void DMAC::DoSIF1Transfer() {
    DMAChannel& channel = channels[6];

    if (channel.quadword_count) {
        // push data to the sif1 fifo
        u128 data = system.ee.Read<u128>(channel.address);

        common::Log("[DMAC] SIF1 Fifo write %016lx%016lx dstat %08x", data.hi, data.lo, interrupt_status);

        system.sif.WriteSIF1FIFO(data);

        // madr and qwc must be updated as the transfer proceeds
        channel.address += 16;
        channel.quadword_count--;
    } else if (channel.end_transfer) {
        EndTransfer(6);
    } else {
        DoSourceChain(6);
    }
}

void DMAC::StartTransfer(int index) {
    common::Log("[DMAC] %s start transfer", channel_names[index]);

    // in normal mode we shouldn't worry about dmatag reading
    u8 mode = (channels[index].control >> 2) & 0x3;
    channels[index].end_transfer = mode == 0;
}

void DMAC::EndTransfer(int index) {
    common::Log("[DMAC] %s end transfer", channel_names[index]);

    channels[index].end_transfer = false;
    channels[index].control &= ~(1 << 8);

    // raise the stat flag in the stat register
    interrupt_status |= (1 << index);

    CheckInterruptSignal();
}

void DMAC::DoSourceChain(int index) {
    DMAChannel& channel = channels[index];
    u128 data = system.ee.Read<u128>(channel.tag_address);
    u64 dma_tag = data.lo;

    common::Log("[DMAC] %s read DMATag %016lx d stat %08x", channel_names[index], dma_tag, interrupt_status);

    channel.quadword_count = dma_tag & 0xFFFF;
    channel.control = (channel.control & 0xFFFF) | (dma_tag & 0xFFFF0000);

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
        common::Error("[DMAC] %s handle DMATag id %d", channel_names[index], id);
    }

    bool irq = (dma_tag >> 31) & 0x1;
    bool tie = (channel.control >> 7) & 0x1;

    if (irq && tie) {
        channel.end_transfer = true;
    }
}