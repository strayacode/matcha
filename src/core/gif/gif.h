#pragma once

#include "common/types.h"
#include "common/log.h"
#include "common/int128.h"
#include <queue>

class System;

// the gif, also known as the graphical interface, is a component of the ps2
// which allows textures and geometry to be sent to the gs for rasterization.
// there are 3 possible paths that can be used to receive data.
// however only 1 path can be ran at a time.
// PATH1: data is transferred via the vu1 using the xgkick instruction
// PATH2: data is transferred via the vif1
// PATH3: data is transferred using the ee via dmac
class GIF {
public:
    GIF(System& system);

    void Reset();
    void SystemReset();

    u32 ReadStat();

    void WriteCTRL(u8 data);
    void WriteFIFO(u128 data);

    void SendPath3(u128 data);
    void ProcessPacked(u128 data);

private:
    u8 ctrl;
    u32 stat;

    std::queue<u128> fifo;

    struct GIFTag {
        u32 nloop;
        bool eop;
        bool prim;
        u32 prim_data;
        u32 format;
        u32 nregs;
        u64 reglist;
        u32 reglist_offset;
        int transfers_left;
    } current_tag;

    System& system;
};