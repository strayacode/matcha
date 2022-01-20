#include "common/log_file.h"
#include "core/ee/timers.h"
#include "core/system.h"

Timers::Timers(System& system) : system(system) {}

void Timers::Reset() {
    for (int i = 0; i < 4; i++) {
        channels[i].counter = 0;
        channels[i].control = 0;
        channels[i].compare = 0;
        channels[i].hold = 0;
        channels[i].cycles = 0;
        channels[i].cycles_per_tick = 0;
    }
}

void Timers::Run(int cycles) {
    while (cycles--) {
        for (int i = 0; i < 4; i++) {
            channels[i].cycles += cycles;

            if (channels[i].control & (1 << 7)) {
                Increment(i);
            }
        }
    }
}

void Timers::WriteRegister(u32 addr, u32 data) {
    int index = (addr >> 11) & 0x3;

    switch (addr & 0xFF) {
    case 0x00:
        log_debug("[Timer] T%d TN_COUNT write %04x", index, data);
        channels[index].counter = data;
        break;
    case 0x10:
        log_debug("[Timer] T%d TN_MODE write %04x", index, data);
        channels[index].control = data;

        // writing 1 to bit 10 or 11 clears them
        if (data & (1 << 10)) {
            channels[index].control &= ~(1 << 10);
        }

        if (data & (1 << 11)) {
            channels[index].control &= ~(1 << 11);
        }

        // update how many timer cycles are required to increment the corresponding channel
        // counter by 1
        switch (channels[index].control & 0x3) {
        case 0:
            // bus clock
            channels[index].cycles_per_tick = 1;
            break;
        case 1:
            // bus clock / 16
            channels[index].cycles_per_tick = 16;
            break;
        case 2:
            // bus block / 256
            channels[index].cycles_per_tick = 256;
            break;
        case 3:
            // hblank
            channels[index].cycles_per_tick = 9370;
            break;
        }

        break;
    case 0x20:
        log_debug("[Timer] T%d TN_COMP write %04x", index, data);
        channels[index].compare = data;
        break;
    case 0x30:
        log_debug("[Timer] T%d TN_HOLD write %04x", index, data);
        channels[index].hold = data;
        break;
    default:
        log_fatal("handle %08x", addr & 0xFF);
    }
}

u32 Timers::ReadChannel(u32 addr) {
    int reg = (addr >> 4) & 0xF;

    switch (reg) {
    default:
        log_fatal("handle reg %d", reg);
    }

    return 0;
}

void Timers::Increment(int index) {
    // if enough ticks have passed then we can increment the timer counter by 1
    if (channels[index].cycles >= channels[index].cycles_per_tick) {
        channels[index].cycles -= channels[index].cycles_per_tick;
    } else {
        // don't do any further calculations as
        // counter wouldn't have changed
        return;
    }

    channels[index].counter++;

    if (channels[index].counter == channels[index].compare) {
        if (channels[index].control & (1 << 6)) {
            channels[index].counter = 0;
        }

        if (channels[index].control & (1 << 8)) {
            log_fatal("handle timer interrupt with compare");
        }
    }

    if (channels[index].counter > 0xFFFF) {
        channels[index].counter = 0;

        // timer interrupts are edge triggered,
        // meaning they can only be requested
        // if either interrupt bit goes from 0 to 1
        if ((channels[index].control & (1 << 9)) && !(channels[index].control & (1 << 11))) {
            // set the overflow interrupt flag and request a timer interrupt
            channels[index].control |= (1 << 11);
            
            LogFile::Get().Log("[Timer] T%d request overflow interrupt\n", index);

            switch (index) {
            case 0:
                system.ee_intc.RequestInterrupt(EEInterruptSource::Timer0);
                break;
            case 1:
                system.ee_intc.RequestInterrupt(EEInterruptSource::Timer1);
                break;
            case 2:
                system.ee_intc.RequestInterrupt(EEInterruptSource::Timer2);
                break;
            case 3:
                system.ee_intc.RequestInterrupt(EEInterruptSource::Timer3);
                break;
            }
        }
    }
}