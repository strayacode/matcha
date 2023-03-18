#pragma once

#include "common/types.h"
#include "core/iop/sio2.h"

struct System;

namespace iop {

class DMAC {
public:
    DMAC(System& system, SIO2& sio2);

    void Reset();
    void Run(int cycles);
    u32 ReadRegister(u32 addr);
    u32 ReadChannel(u32 addr);
    void WriteRegister(u32 addr, u32 data);
    void WriteChannel(u32 addr, u32 data);
    int GetChannelIndex(u32 addr);
    bool GetChannelEnable(int index);
    void DoSIF0Transfer();
    void DoSIF1Transfer();
    void DoSPU2Transfer();
    void DoSIO2InTransfer();
    void DoSIO2OutTransfer();
    void EndTransfer(int index);

    // dma priority/enable
    // used for the first 7 channels
    u32 dpcr;

    // dma priority/enable 2
    // used for the remaining channels
    u32 dpcr2;

    union DICR {
        struct {
            u8 completion: 7;
            u32 : 8;
            bool force_irq : 1;
            u8 masks : 7;
            bool master_interrupt_enable : 1;
            u8 flags : 7;
            bool master_interrupt_flag : 1;
        };

        u32 data;
    } dicr;

    union DICR2 {
        struct {
            u16 tag_interrupt : 13;
            u8 : 2;
            bool force_irq : 1;
            u8 masks : 6;
            u8 : 2;
            u8 flags : 6;
            u8 : 2;
        };

        u32 data;
    } dicr2;

    struct Channel {
        u32 address;
        u16 block_size; // in words (0 = 0x10000)
        u16 block_count;
        u32 control;
        u32 tag_address;
        bool end_transfer;
    } channels[13];

    bool global_dma_enable;
    bool global_dma_interrupt_control;

private:
    System& system;
    SIO2& sio2;
};

} // namespace iop