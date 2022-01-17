#pragma once

#include "common/types.h"

class IOPDMAC {
public:
    void Reset();
    u32 ReadRegister(u32 addr);
    void WriteRegister(u32 addr, u32 data);
    void WriteChannel(u32 addr, u32 data);
    int GetChannelIndex(u32 addr);

    // dma priority/enable
    // used for the first 7 channels
    u32 dpcr;

    // dma priority/enable 2
    // used for the remaining channels
    u32 dpcr2;

    u32 dicr;
    u32 dicr2;

    struct Channel {
        u32 address;
        u16 block_size; // in words (0 = 0x10000)
        u16 block_count;
        u32 control;
        u32 tag_address;
    } channels[13];

    bool global_dma_enable;
    bool global_dma_interrupt_control;

private:

};