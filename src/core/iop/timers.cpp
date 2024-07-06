#include "common/log.h"
#include "core/iop/timers.h"

namespace iop {

Timers::Timers(INTC& intc) : m_intc(intc) {}

void Timers::reset() {
    for (int i = 0; i < 6; i++) {
        m_channels[i].counter = 0;
        m_channels[i].cycles = 0;
        m_channels[i].cycles_per_tick = 1;
        m_channels[i].mode.data = 0;
        m_channels[i].target = 0;
    }
}

// TODO: put timers on scheduler for performance
void Timers::run(int cycles) {
    for (int i = 0; i < 6; i++) {
        run_channel(i, cycles);
    }
}

u32 Timers::read(u32 addr) {
    int index = calculate_channel_index(addr);
    auto& channel = m_channels[index];

    switch (addr & 0xf) {
    case 0x0:
        common::Log("[iop::Timers] channel %d counter read %08x", index, static_cast<u32>(channel.counter));
        return channel.counter;
    case 0x4:
        // reads clear the two raised interrupt flags
        channel.mode.compare_irq_raised = false;
        channel.mode.overflow_irq_raised = false;

        common::Log("[iop::Timers] channel %d mode read %08x", index, channel.mode.data);
        return channel.mode.data;
    case 0x8:
        common::Log("[iop::Timers] channel %d target read %08x", index, channel.target);
        return channel.target;
    default:
        LOG_TODO("unknown timer read %08x", addr);
    }
}

void Timers::write(u32 addr, u32 data) {
    int index = calculate_channel_index(addr);
    auto& channel = m_channels[index];
    
    switch (addr & 0xf) {
    case 0x0:
        common::Log("[iop::Timers] channel %d counter write %08x", index, data);
        channel.counter = data;
        break;
    case 0x4: {
        common::Log("[iop::Timers] channel %d mode write %08x", index, data);
        channel.mode.data = data;

        if (channel.mode.gate_enable) {
            LOG_TODO_NO_ARGS("handle gate enabled");
        }

        // Enable interrupts.
        channel.mode.irqs_enabled = true;

        // Calculate clock source.
        // TODO: write hardware test for this later.
        if (channel.mode.use_external_signal) {
            switch (index) {
            case 0:
                LOG_TODO_NO_ARGS("handle pixel clock source");
            case 1:
            case 3:
                // TODO: is hblank every 2350 cycles?
                channel.cycles_per_tick = 2350;
                break;
            default:
                channel.cycles_per_tick = 1;
            }
        }

        // Get prescaler.
        u32 prescaler = calculate_channel_prescaler(index);

        // Multiply cycles per tick with prescaler.
        channel.cycles_per_tick *= prescaler;

        common::Log("timer %d prescaler set to %d cycles per tick %d", index, prescaler, channel.cycles_per_tick);

        channel.counter = 0;
        channel.cycles = 0;
        break;
    }
    case 0x8:
        common::Log("[iop::Timers] channel %d target write %08x", index, data);
        channel.target = data;

        if (index < 4) {
            // First 3 channels only use first 16 bits of target.
            channel.target &= 0xffff;
        }

        // If bit 7 of mode is not set,
        // then writes to target set bit 10 of
        // mode to 1.
        if (!channel.mode.levl) {
            channel.mode.irqs_enabled = true;
        }

        break;
    default:
        LOG_TODO("unknown timer write %08x", addr);
    }
}

int Timers::calculate_channel_index(u32 addr) {
    int index = (addr >> 4) & 0xf;
    if (index >= 8) {
        return index - 5;
    } else {
        return index;
    }
}

u32 Timers::calculate_channel_prescaler(int index) {
    auto& channel = m_channels[index];

    if (index == 2 && channel.mode.timer2_prescaler) {
        return 8;
    } else if (index >= 4) {
        switch (channel.mode.timer45_prescaler) {
        case 0:
            return 1;
        case 1:
            return 8;
        case 2:
            return 16;
        case 3:
            return 256;
        default:
            return 1;
        }
    } else {
        return 1;
    }
}

void Timers::run_channel(int index, int cycles) {
    auto& channel = m_channels[index];
    channel.cycles += cycles;

    while (channel.cycles >= channel.cycles_per_tick) {
        channel.cycles -= channel.cycles_per_tick;
        channel.counter++;

        // Check for timer overflows.
        if (overflow_occured(index) && channel.mode.overflow_irq && !channel.mode.overflow_irq_raised) {
            common::Log("[iop::Timers] overflow irq occured for channel %d with counter %08x", index, static_cast<u32>(channel.counter));
            raise_irq(index);
            channel.mode.overflow_irq_raised = true;
        }

        // Mask counter and check for compare interrupts.
        if (index < 3) {
            channel.counter &= 0xffff;
        } else {
            channel.counter &= 0xffffffff;
        }

        if (channel.counter == channel.target && channel.mode.compare_irq && !channel.mode.compare_irq_raised) {
            common::Log("[iop::Timers] compare irq occured for channel %d with counter %08x", index, static_cast<u32>(channel.counter));
            raise_irq(index);
            channel.mode.compare_irq_raised = true;

            if (channel.mode.zero_return) {
                channel.counter = 0;
            }
        }
    }
}

bool Timers::overflow_occured(int index) {
    auto& channel = m_channels[index];
    if (index < 3) {
        // Channels 0..2 use 16-bit counter.
        return channel.counter > 0xffff;
    } else {
        // Channels 3..5 use 32-bit counter.
        return channel.counter > 0xffffffff;
    }
}

void Timers::raise_irq(int index) {
    auto& channel = m_channels[index];
    int irq = index < 3 ? 4 + index : 11 + index;

    m_intc.RequestInterrupt(static_cast<InterruptSource>(irq));

    if (!channel.mode.repeat_irq) {
        common::Log("[iop::Timers] repeat irq disabled for channel %d, so disable irq", index);
        channel.mode.irqs_enabled = false;
    } else if (channel.mode.levl) {
        common::Log("[iop::Timers] toggle irq enabled for channel %d, so toggle irq", index);
        channel.mode.irqs_enabled ^= true;
    }
}

} // namespace iop