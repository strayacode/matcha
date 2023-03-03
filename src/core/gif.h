#pragma once

#include "common/types.h"
#include "common/log.h"
#include "common/queue.h"

struct System;

// the gif, also known as the graphical interface, is a component of the ps2
// which allows textures and geometry to be sent to the gs for rasterization.
// there are 3 possible paths that can be used to receive data.
// however only 1 path can be ran at a time.
// PATH1: data is transferred via the vu1 using the xgkick instruction
// PATH2: data is transferred via the vif1
// PATH3: data is transferred using the ee via dmac
struct GIF {
    GIF(System& system);

    void Reset();
    void SystemReset();
    void Run(int cycles);

    u32 ReadRegister(u32 addr);
    void WriteRegister(u32 addr, u32 value);

    void WriteFIFO(u32 value);

    void SendPath3(u128 value);
    void ProcessPacked(u128 data);
    void ProcessImage(u128 data);

private:
    void StartTransfer();
    void ProcessTag();

    u8 ctrl;
    u32 stat;

    // the path3 fifo stores up to 16 quadwords (128-bit)
    common::Queue<u32, 64> fifo;
    
    struct Tag {
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