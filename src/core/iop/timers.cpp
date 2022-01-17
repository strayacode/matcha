#include "common/log.h"
#include "core/iop/timers.h"

void IOPTimers::Reset() {
    for (int i = 0; i < 5; i++) {
        channels[i].counter = 0;
        channels[i].mode = 0;
        channels[i].target = 0;
    }
}

u32 IOPTimers::ReadRegister(u32 addr) {
    int channel = GetTimerIndex(addr);
    int index = addr & 0xF;

    switch (index) {
    case 0x0:
        return channels[channel].counter;
    case 0x8:
        return channels[channel].target;
    default:
        log_fatal("handle %02x", index);
    }
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
        break;
    case 0x8:
        channels[channel].target = data;
        break;
    default:
        log_fatal("handle %02x", index);
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