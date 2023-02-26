#pragma once

#include "common/types.h"
#include "common/log.h"

enum class DMAChannelType : int {
    VIF0 = 0,
    VIF1 = 1,
    GIF = 2,
    IPUFrom = 3,
    IPUTo = 4,
    SIF0 = 5,
    SIF1 = 6,
    SIF2 = 7,
    SPRFrom = 8,
    SPRTo = 9,
};

struct DMAChannel {
    u32 control;
    u32 address;
    u32 tag_address;
    u32 quadword_count;
    u32 saved_tag_address0;
    u32 saved_tag_address1;
    u32 scratchpad_address;
    bool end_transfer;
};

class System;

class DMAC {
public:
    DMAC(System* system);

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

    void DoGIFTransfer();
    void DoSIF0Transfer();
    void DoSIF1Transfer();

    void StartTransfer(int index);
    void EndTransfer(int index);
    
    int GetChannelIndex(u32 addr);
    void CheckInterruptSignal();

    void DoSourceChain(int index);

    DMAChannel channels[10];

    u32 control;
    u32 interrupt_status;
    u32 priority_control;
    u32 skip_quadword;
    u32 ringbuffer_size;
    u32 ringbuffer_offset;
    u32 disabled_status;

    System* system;
};