#pragma once

#include <queue>
#include "common/types.h"
#include "common/log.h"
#include "common/queue.h"

class SIF {
public:
    void Reset();

    void WriteEEControl(u32 data);
    void WriteIOPControl(u32 data);
    void WriteBD6(u32 data);
    void WriteMSCOM(u32 data);
    void WriteSMCOM(u32 data);
    void SetMSFLAG(u32 data);
    void SetSMFLAG(u32 data);
    void ResetMSFLAG(u32 data);
    void ResetSMFLAG(u32 data);

    u32 ReadMSFLAG();
    u32 ReadMSCOM();
    u32 ReadSMFLAG();
    u32 ReadSMCOM();
    u32 ReadControl();

    // no clue what this is for
    u32 control;
    u32 bd6;

    // only writeable by the ee
    u32 mscom;

    // only writeable by the iop
    u32 smcom;

    u32 msflag;
    u32 smflag;

    // TODO: do more research into sif dmas and sif fifo
    std::queue<u32> sif0_fifo;
    std::queue<u32> sif1_fifo;

    u32 ReadSIF0FIFO();
    u32 ReadSIF1FIFO();
    void write_sif0_fifo(u32 data);
    void write_sif1_fifo(u128 data);
    int GetSIF0FIFOSize();
    int GetSIF1FIFOSize();
};