#pragma once

#include "common/types.h"

struct System;

namespace ee {

class DMAC {
public:
    DMAC(System& system);

    void Reset();
    void Run(int cycles);

    void WriteRegister(u32 addr, u32 data);

    u32 ReadChannel(u32 addr);
    void WriteChannel(u32 addr, u32 data);
    u32 ReadInterruptStatus();
    void WriteControl(u32 data);
    u32 ReadControl();
    u32 ReadPriorityControl();
    u32 ReadSkipQuadword();

    void Transfer(int index);

    void do_gif_transfer();
    void do_sif0_transfer();
    void do_sif1_transfer();
    void do_to_spr_transfer();

    void StartTransfer(int index);
    void EndTransfer(int index);
    
    int GetChannelIndex(u32 addr);
    void CheckInterruptSignal();

    void DoSourceChain(int index);

    u32 control;
    u32 interrupt_status;
    u32 priority_control;
    u32 skip_quadword;
    u32 ringbuffer_size;
    u32 ringbuffer_offset;
    u32 disabled_status;

private:
    // TODO: dmac specifies transfer addresses as physical addresses, skipping the TLB.
    // We should replace all ee memory accesses with a specific function
    u128 read_u128(u32 addr);
    void write_u128(u32 addr, u128 data);

    struct Channel {
        enum class Mode : u8 {
            Normal = 0,
            Chain = 1,
            Interleave = 2,
        };

        union Control {
            struct {
                // Only used for vif1 and sif2 transfers.
                bool from_memory : 1;

                u32 : 1;
                Mode mode : 2;
                u32 address_stack_pointer : 2;

                // Only used in source chain mode.
                bool transfer_dmatag : 1;

                bool dmatag_irq : 1;
                bool busy : 1;
                u32 : 7;
                u32 dmatag_upper : 16;
            };

            u32 data;
        } control;

        u32 address;
        u32 tag_address;
        u32 quadword_count;
        u32 saved_tag_address0;
        u32 saved_tag_address1;
        u32 scratchpad_address;
        bool end_transfer;
    };

    enum class ChannelType : int {
        VIF0 = 0,
        VIF1 = 1,
        GIF = 2,
        IPUFrom = 3,
        IPUTo = 4,
        SIF0 = 5,
        SIF1 = 6,
        SIF2 = 7,
        FromSPR = 8,
        ToSPR = 9,
    };

    Channel channels[10];
    System& system;
};

} // namespace ee