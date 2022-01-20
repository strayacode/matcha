#include "core/ee/dmac.h"
#include "core/system.h"

static const char* channel_names[10] = {
    "VIF0", "VIF1", "GIF", "IPU_FROM", "IPU_TO",
    "SIF0", "SIF1", "SIF2", "SPR_FROM", "SPR_TO",
};

DMAC::DMAC(System* system) : system(system) {

}

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
        // log_debug("[DMAC %d] control read %08x", index, channels[index].control);
        return channels[index].control;
    case 0x10:
        return channels[index].address;
    case 0x20:
        return channels[index].quadword_count;
    case 0x30:
        return channels[index].tag_address;
    default:
        log_fatal("[DMAC] Handle %02x", addr & 0xFF);
    }
}

void DMAC::WriteRegister(u32 addr, u32 data) {
    switch (addr) {
    case 0x1000E000:
        log_debug("[DMAC] D_CTRL write %08x", data);
        control = data;
        break;
    case 0x1000E010:
        log_debug("[DMAC] D_STAT write %08x", data);

        // for bits (0..9) they get cleared if 1 is written
        interrupt_status &= ~(data & 0x3FF);

        // for bits (16..25) they get reversed if 1 is written
        interrupt_status ^= (data & 0x3FF0000);

        CheckInterruptSignal();
        break;
    case 0x1000E020:
        log_debug("[DMAC] D_PCR write %08x", data);
        priority_control = data;
        break;
    case 0x1000E030:
        log_debug("[DMAC] D_SQWC write %08x", data);
        skip_quadword = data;
        break;
    case 0x1000E040:
        log_debug("[DMAC] D_RBSR write %08x", data);
        ringbuffer_size = data;
        break;
    case 0x1000E050:
        log_debug("[DMAC] D_RBOR write %08x", data);
        ringbuffer_offset = data;
        break;
    case 0x1000F590:
        log_debug("[DMAC] D_ENABLE write %08x", data);
        disabled_status = data;
        break;
    default:
        if (addr >= 0x1000E000) {
            log_fatal("[DMAC] handle write %08x = %08x", addr, data);
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
        log_debug("[DMAC] %s Dn_CHCR write %08x", channel_name, data);
        channels[index].control = data;
        break;
    case 0x10:
        log_debug("[DMAC] %s Dn_MADR write %08x", channel_name, data);
        channels[index].address = data & ~0xF;
        break;
    case 0x20:
        // In normal and interleaved mode, the transfer ends when QWC reaches zero. Chain mode behaves differently
        log_debug("[DMAC] %s Dn_QWC write %08x", channel_name, data);
        channels[index].quadword_count = data & 0xFFFF;
        break;
    case 0x30:
        log_debug("[DMAC] %s Dn_TADR write %08x", channel_name, data);
        channels[index].tag_address = data & ~0xF;
        break;
    case 0x40:
        log_debug("[DMAC] %s Dn_ASR0 write %08x", channel_name, data);
        channels[index].saved_tag_address0 = data & ~0xF;
        break;
    case 0x50:
        log_debug("[DMAC] %s Dn_ASR1 write %08x", channel_name, data);
        channels[index].saved_tag_address1 = data & ~0xF;
        break;
    case 0x80:
        log_debug("[DMAC] %s Dn_SADR write %08x", channel_name, data);
        channels[index].scratchpad_address = data & ~0xF;
        break;
    default:
        log_fatal("[DMAC] Handle channel with identifier %02x and data %08x", addr & 0xFF, data);
    }
}

int DMAC::GetChannelIndex(u32 addr) {
    u8 channel_identifier = (addr >> 8) & 0xFF;

    DMAChannelType index;

    switch (channel_identifier) {
    case 0x80:
        index = DMAChannelType::VIF0;
        break;
    case 0x90:
        index = DMAChannelType::VIF1;
        break;
    case 0xA0:
        index = DMAChannelType::GIF;
        break;
    case 0xB0:
        index = DMAChannelType::IPUFrom;
        break;
    case 0xB4:
        index = DMAChannelType::IPUTo;
        break;
    case 0xC0:
        index = DMAChannelType::SIF0;
        break;
    case 0xC4:
        index = DMAChannelType::SIF1;
        break;
    case 0xC8:
        index = DMAChannelType::SIF2;
        break;
    case 0xD0:
        index = DMAChannelType::SPRFrom;
        break;
    case 0xD4:
        index = DMAChannelType::SPRTo;
        break;
    default:
        log_fatal("[DMAC] Random behaviour!");
    }

    return static_cast<int>(index);
}

u32 DMAC::ReadInterruptStatus() {
    log_debug("[DMAC] D_STAT read %08x", interrupt_status);
    return interrupt_status;
}

u32 DMAC::ReadControl() {
    log_debug("[DMAC] read control %08x", control);
    return control;
}

u32 DMAC::ReadPriorityControl() {
    log_warn("[DMAC] read priority control %08x", priority_control);
    return priority_control;
}

u32 DMAC::ReadSkipQuadword() {
    log_warn("[DMAC] read skip quadword %08x", skip_quadword);
    return skip_quadword;
}

void DMAC::CheckInterruptSignal() {
    bool irq = false;

    for (int i = 0; i < 10; i++) {
        if ((interrupt_status & (1 << i)) && (interrupt_status & (1 << (16 + i)))) {
            irq = true;
            break;
        }
    }

    system->ee_core.SendInterruptSignal(1, irq);
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
    case DMAChannelType::SIF0:
        DoSIF0Transfer();
        break;
    case DMAChannelType::SIF1:
        DoSIF1Transfer();
        break;
    default:
        log_fatal("handle %d", index);
    }
}

void DMAC::DoSIF0Transfer() {
    DMAChannel& channel = channels[5];

    if (channel.quadword_count) {
        // dmac can only transfer a quadword at a time
        if (system->sif.GetSIF0FIFOSize() >= 4) {
            for (int i = 0; i < 4; i++) {
                system->ee_core.WriteWord(channel.address, system->sif.ReadSIF0FIFO());
                channel.address += 4;
            }

            channel.quadword_count--;
        }
    } else if (channel.end_transfer) {
        EndTransfer(5);
    } else {
        if (system->sif.GetSIF0FIFOSize() >= 2) {
            // form a dmatag
            u64 dma_tag = 0;

            dma_tag |= system->sif.ReadSIF0FIFO();
            dma_tag |= (u64)system->sif.ReadSIF0FIFO() << 32;
            channel.quadword_count = dma_tag & 0xFFFF;
            channel.address = (dma_tag >> 32) & 0xFFFFFFF0;
            channel.tag_address += 16;
            channel.control = (channel.control & 0xFFFF) | (dma_tag & 0xFFFF0000);

            if (((dma_tag & (1 << 31)) && (channel.control & (1 << 7)))) {
                channel.end_transfer = true;
            }
        }
    }
}

void DMAC::DoSIF1Transfer() {
    DMAChannel& channel = channels[6];

    if (channel.quadword_count) {
        // push data to the sif1 fifo
        system->sif.WriteSIF1FIFO(system->ee_core.ReadQuad(channel.address));

        // madr and qwc must be updated as the transfer proceeds
        channel.address += 16;
        channel.quadword_count--;
    } else if (channel.end_transfer) {
        EndTransfer(6);
    } else {
        DoSourceChain(6);
    }
}

void DMAC::EndTransfer(int index) {
    channels[index].end_transfer = false;
    channels[index].control &= ~(1 << 8);

    // raise the stat flag in the stat register
    interrupt_status |= (1 << index);

    CheckInterruptSignal();
}

void DMAC::DoSourceChain(int index) {
    DMAChannel& channel = channels[index];
    u128 data = system->ee_core.ReadQuad(channel.tag_address);
    u64 dma_tag = data.i.lo;

    // log_debug("[DMAC] %s read dmatag %016lx from tag address %08x", channel_names[index], dma_tag, channel.tag_address);

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
    default:
        log_fatal("[DMAC] %s handle DMATag id %d", channel_names[index], id);
    }

    if (((dma_tag & (1 << 31)) && (channel.control & (1 << 7)))) {
        channel.end_transfer = true;
    }
}