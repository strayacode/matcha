#include <core/ee/dmac.h>
#include <core/system.h>

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
    }

    control = 0;
    interrupt_status = 0;
    priority_control = 0;
    skip_quadword = 0;
    ringbuffer_size = 0;
    ringbuffer_offset = 0;
    disabled_status = 0x1201;
    disable = 0;
}

u32 DMAC::ReadChannel(u32 addr) {
    int index = GetChannelIndex(addr);

    switch (addr & 0xFF) {
    case 0x00:
        log_warn("[DMAC %d] control read %08x", index, channels[index].control);
        return channels[index].control;
    case 0x20:
        return channels[index].quadword_count;
    case 0x30:
        return channels[index].tag_address;
    default:
        log_fatal("[DMAC] Handle %02x", addr & 0xFF);
    }
}

void DMAC::WriteChannel(u32 addr, u32 data) {
    int index = GetChannelIndex(addr);

    switch (addr & 0xFF) {
    case 0x00:
        // TODO: mask bits properly
        log_warn("[DMAC %d] control write %08x", index, data);
        channels[index].control = data;
        break;
    case 0x10:
        // lower bits must be 0
        log_warn("[DMAC %d] address write %08x", index, data);
        if ((data & 0xF) != 0) {
            log_warn("[DMAC] Lower 4 bits aren't 0");
        }
        
        channels[index].address = data;
        break;
    case 0x20:
        // In normal and interleaved mode, the transfer ends when QWC reaches zero. Chain mode behaves differently
        log_warn("[DMAC %d] quadword count write %08x", index, data);
        channels[index].quadword_count = data;
        break;
    case 0x30:
        // lower bits must be 0
        log_warn("[DMAC %d] tag address write %08x", index, data);
        if ((data & 0xF) != 0) {
            log_warn("[DMAC] Lower 4 bits aren't 0");
        }
        channels[index].tag_address = data;
        break;
    case 0x40:
        // lower bits must be 0
        log_warn("[DMAC %d] saved tag address0 write %08x", index, data);
        if ((data & 0xF) != 0) {
            log_warn("[DMAC] Lower 4 bits aren't 0");
        }
        channels[index].saved_tag_address0 = data;
        break;
    case 0x50:
        // lower bits must be 0
        log_warn("[DMAC %d] saved tag address1 write %08x", index, data);
        if ((data & 0xF) != 0) {
            log_warn("[DMAC] Lower 4 bits aren't 0");
        }
        channels[index].saved_tag_address1 = data;
        break;
    case 0x80:
        // lower bits must be 0
        log_warn("[DMAC %d] scratchpad address write %08x", index, data);
        if ((data & 0xF) != 0) {
            log_warn("[DMAC] Lower 4 bits aren't 0");
        }
        channels[index].scratchpad_address = data;
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

void DMAC::WriteInterruptStatus(u32 data) {
    log_warn("[DMAC] write interrupt status %08x", data);
    // writing 1 to stat clears an interrupt
    // writing 1 to mask reverses an interrupt
    for (int i = 0; i < 10; i++) {
        if (data & (1 << i)) {
            // clear that bit in interrupt_status
            interrupt_status &= ~(1 << i);
        }

        if (data & (1 << (16 + i))) {
            // reverse that bit in interrupt_status
            interrupt_status ^= (1 << i);
        }
    }

    interrupt_status &= ~(data & (1 << 13));
    interrupt_status &= ~(data & (1 << 14));
    interrupt_status &= ~(data & (1 << 15));

    if (data & (1 << 29)) {
        interrupt_status ^= (1 << 29);
    }

    if (data & (1 << 30)) {
        interrupt_status ^= (1 << 30);
    }

    CheckInterruptSignal();
}

u32 DMAC::ReadInterruptStatus() {
    log_warn("[DMAC] read interrupt status %08x", interrupt_status);
    return interrupt_status;
}

void DMAC::WriteControl(u32 data) {
    log_warn("[DMAC] write control %08x", data);
    control = data;
}

u32 DMAC::ReadControl() {
    log_warn("[DMAC] read control %08x", control);
    return control;
}

void DMAC::WritePriorityControl(u32 data) {
    log_warn("[DMAC] write priority control %08x", data);
    priority_control = data;
}

u32 DMAC::ReadPriorityControl() {
    log_warn("[DMAC] read priority control %08x", priority_control);
    return priority_control;
}

void DMAC::WriteSkipQuadword(u32 data) {
    log_warn("[DMAC] write skip quadword %08x", data);
    skip_quadword = data;
}

u32 DMAC::ReadSkipQuadword() {
    log_warn("[DMAC] read skip quadword %08x", skip_quadword);
    return skip_quadword;
}

void DMAC::WriteRingBufferSize(u32 data) {
    log_warn("[DMAC] write ring buffer size %08x", data);
    ringbuffer_size = data;
}

void DMAC::WriteRingBufferOffset(u32 data) {
    log_warn("[DMAC] write ring buffer offset %08x", data);
    ringbuffer_offset = data;
}

void DMAC::CheckInterruptSignal() {
    // for each pair of bits if there is an irq and its enabled,
    // then send an INT1 signal to the ee
    for (int i = 0; i < 10; i++) {
        if ((interrupt_status & (1 << i)) && (interrupt_status & (1 << (16 + i)))) {
            log_fatal("[DMAC] Handle int1 signal!");
        }
    }
}

// note:
// dmac can transfer one quadword (16 bytes / 128 bits) per bus cycle
void DMAC::Run(int cycles) {
    // don't run anything if the dmac is not enabled
    if (!(control & 0x1)) {
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
    default:
        log_fatal("handle %d", index);
    }
}

void DMAC::DoSIF0Transfer() {
    log_debug("do sif0 transfer %08x", system->ee_core.pc);
    DMAChannel& channel = channels[5];

    if (channel.quadword_count) {
        log_fatal("handle");
    } else {
        EndTransfer(5);
    }
}

void DMAC::DoSIF1Transfer() {

}

void DMAC::EndTransfer(int index) {
    channels[index].control &= ~(1 << 8);

    // raise the stat flag in the stat register
    interrupt_status |= (1 << index);
    CheckInterruptSignal();
}