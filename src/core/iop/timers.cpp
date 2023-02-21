#include "common/log.h"
#include "common/log.h"
#include "core/iop/timers.h"
#include "core/system.h"

IOPTimers::IOPTimers(System& system) : system(system) {}

void IOPTimers::Reset() {
    for (int i = 0; i < 5; i++) {
        channels[i].counter = 0;
        channels[i].mode = 0;
        channels[i].target = 0;
    }
}

// TODO: handle repeat bit
void IOPTimers::Run(int cycles) {
    // only timer 5 is required for now it seems
    Channel& channel = channels[5];

    u32 old_counter = channel.counter;
    channel.counter += cycles;

    if ((channel.counter >= channel.target) && (old_counter < channel.target)) {
        channel.mode |= (1 << 11);

        if ((channel.mode & (1 << 4)) && (channel.mode & (1 << 10))) {
            common::Log("[IOP Timers] channel 5 send timer interrupt");
            system.iop_core->interrupt_controller.RequestInterrupt(IOPInterruptSource::Timer5);

            if ((channel.mode & (1 << 6)) == 0) {
                // bit 10 of mode is set to 0 after an interrupt occurs
                channel.mode &= ~(1 << 10);
            }
        }

        if (channel.mode & (1 << 3)) {
            channel.counter = 0;
        }
    }

    if (channel.counter > 0xFFFF) {
        channel.mode |= (1 << 12);

        if ((channel.mode & (1 << 5)) && (channel.mode & (1 << 10))) {
            common::Log("[IOP Timers] channel 5 send timer interrupt");
            system.iop_core->interrupt_controller.RequestInterrupt(IOPInterruptSource::Timer5);

            if ((channel.mode & (1 << 6)) == 0) {
                // bit 10 of mode is set to 0 after an interrupt occurs
                channel.mode &= ~(1 << 10);
            }
        }

        channel.counter = 0;
    }
}

u32 IOPTimers::ReadRegister(u32 addr) {
    int channel = GetTimerIndex(addr);
    int index = addr & 0xF;

    switch (index) {
    case 0x0:
        return channels[channel].counter;
    case 0x4:
        // reads clear the two raised interrupt flags
        channels[channel].mode &= ~(1 << 11);
        channels[channel].mode &= ~(1 << 12);

        return channels[channel].mode;
    case 0x8:
        return channels[channel].target;
    default:
        common::Error("handle %02x", index);
    }

    return 0;
}

void IOPTimers::WriteRegister(u32 addr, u32 data) {
    int channel = GetTimerIndex(addr);
    int index = addr & 0xF;

    switch (index) {
    case 0x0:
        channels[channel].counter = data;
        break;
    case 0x4:
        channels[channel].mode = data;

        // mode writes set counter to 0 and bit 10 to 1
        channels[channel].counter = data;
        channels[channel].mode |= (1 << 10);
        break;
    case 0x8:
        channels[channel].target = data;

        // biit 7 of mode is not set,
        // then writes to target set bit 10 of
        // mode to 1
        if ((channels[channel].mode & (1 << 7)) == 0) {
            channels[channel].mode |= (1 << 10);
        }

        break;
    default:
        common::Error("handle %02x", index);
    }
}

int IOPTimers::GetTimerIndex(u32 addr) {
    int channel = (addr >> 4) & 0xF;

    if (channel >= 8) {
        return channel - 5;
    } else {
        return channel;
    }
}