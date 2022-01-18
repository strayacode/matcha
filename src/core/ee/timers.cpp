#include "core/ee/timers.h"
#include "core/system.h"

Timers::Timers(System& system) : system(system) {}

void Timers::Reset() {
    for (int i = 0; i < 4; i++) {
        channels[i].counter = 0;
        channels[i].control = 0;
        channels[i].compare = 0;
        channels[i].hold = 0;
        channels[i].ticks = 0;
    }
}

void Timers::Run(int cycles) {
    // this is slow af
    while (cycles--) {
        for (int i = 0; i < 4; i++) {
            u8 clock = channels[i].control & 0x3;
            if (channels[i].control & (1 << 7)) {
                switch (clock) {
                case 0:
                    // since timers are ran at bus clock speed we can just increment by 1
                    Tick(i, 1);
                    break;
                case 2:
                    // bus speed / 16
                    channels[i].ticks++;

                    if (channels[i].ticks >= 16) {
                        Tick(i, 1);
                        channels[i].ticks = 0;
                    }
                    break;
                case 3:
                    channels[i].ticks++;

                    if (channels[i].ticks >= 9371) {
                        Tick(i, 1);
                        channels[i].ticks = 0;
                    }
                    break;
                default:
                    log_fatal("handle different clock type %d", clock);
                }
            }
        }
    }
}

void Timers::WriteChannel(u32 addr, u32 data) {
    int reg = (addr >> 4) & 0x3;
    int index = (addr >> 11) & 0x3;

    switch (reg) {
    case 0:
        WriteCounter(index, data);
        break;
    case 1:
        WriteControl(index, data);
        break;
    case 2:
        WriteCompare(index, data);
        break;
    case 3:
        WriteHold(index, data);
        break;
    default:
        log_fatal("handle reg %d", reg);
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

void Timers::WriteControl(int index, u16 data) {
    log_warn("[Timer %d] control write %04x", index, data);
    channels[index].control = data;

    // writing 1 to bit 10 or 11 clears them
    if (data & (1 << 10)) {
        channels[index].control &= ~(1 << 10);
    }

    if (data & (1 << 11)) {
        channels[index].control &= ~(1 << 11);
    }
}

void Timers::WriteCounter(int index, u16 data) {
    log_warn("[Timer %d] counter write %04x", index, data);
    channels[index].counter = data;
}

void Timers::WriteCompare(int index, u16 data) {
    log_warn("[Timer %d] compare write %04x", index, data);
    channels[index].compare = data;
}

void Timers::WriteHold(int index, u16 data) {
    log_warn("[Timer %d] hold write %04x", index, data);
    channels[index].hold = data;
}

void Timers::Tick(int index, int ticks) {
    channels[index].counter += ticks;

    if (channels[index].counter == channels[index].compare) {
        // reset counter when counter is same as compare
        if (channels[index].control & (1 << 6)) {
            channels[index].counter = 0;
        }

        if (channels[index].control & (1 << 8)) {
            log_fatal("handle timer interrupt with compare");
        }
    }

    if (channels[index].counter > 0xFFFF) {
        channels[index].counter = 0;

        if (channels[index].control & (1 << 9)) {
            // set the overflow interrupt flag and request 
            channels[index].control |= (1 << 11);

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