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
    m_command_length = 0;
    m_command_index = 0;
    m_peripheral_type = PeripheralType::None;
}

u32 SIO2::ReadRegister(u32 addr) {
    switch (addr) {
    case 0x1f808264:
        // fifo out
        common::Log("[iop::SIO2] fifo read %08x", 0);
        return fifo.Pop<u8>();
    case 0x1f808268:
        common::Log("[iop::SIO2] control read %08x", control);
        return control;
    case 0x1f80826c:
        // response status 1
        // For now just return disconnected peripheral status
        common::Log("[iop::SIO2] recv1 read %08x", 0x1d100);
        return 0x1d100;
    case 0x1f808270:
        // response status 2
        common::Log("[iop::SIO2] recv2 read %08x", 0xf);
        return 0xf;
    case 0x1f808274:
        // response status 3
        common::Log("[iop::SIO2] recv3 read %08x", 0);
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
        upload_command(value);
        break;
    case 0x1f808268:
        control = value;

        if (value & 0x1) {
            intc.RequestInterrupt(InterruptSource::SIO2);
            control &= ~0x1;
        }

        if (value & 0xc) {
            // TODO: reset state here for next transfer
            m_command_length = 0;
            m_command_index = 0;
            m_peripheral_type = PeripheralType::None;
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
    u8 data = fifo.Pop<u8>();
    common::Log("[iop::SIO2] dma read %02x", data);
    return data;
}

void SIO2::WriteDMA(u8 data) {
    common::Log("[iop::SIO2] dma data write %02x", data);
    upload_command(data);
}

void SIO2::upload_command(u8 data) {
    if (m_command_length == 0) {
        // Start new transfer.
        switch (data) {
        case 0x00:
            m_peripheral_type = PeripheralType::None;
            break;
        case 0x01:
            m_peripheral_type = PeripheralType::Controller;
            break;
        case 0x81:
            m_peripheral_type = PeripheralType::Memcard;
            break;
        default:
            LOG_TODO("start new transfer with peripheral byte %02x", data);
        }

        u32 params = send3[m_command_index];
        common::Log("get params %08x from index in send3 %d", params, m_command_index);
        if (params == 0) {
            common::Log("[iop::sio2] params is empty!");
        }

        m_command_index++;
        m_command_length = (params >> 18) & 0x7f;
        common::Log("[iop::sio2] start transfer with length %d", m_command_length);
    }

    m_command_length--;

    switch (m_peripheral_type) {
    case PeripheralType::Controller:
        // Just stub reply for now.
        fifo.Push<u8>(0x00);
        break;
    case PeripheralType::Memcard:
        // Just stub reply for now.
        fifo.Push<u8>(0xff);
        break;
    default:
        common::Log("[iop::sio2] handle non-controller transfer"); 
    }
}

} // namespace iop