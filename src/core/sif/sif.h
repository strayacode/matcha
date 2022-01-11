#pragma once

#include <common/types.h>
#include <common/log.h>

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
    void ResetSMFLAG(u32 data);

    u32 ReadMSFLAG();
    u32 ReadMSCOM();
    u32 ReadSMFLAG();
    u32 ReadSMCOM();
    u32 ReadControl();
private:
    // no clue what this is for
    u32 control;
    u32 bd6;

    // only writeable by the ee
    u32 mscom;

    // only writeable by the iop
    u32 smcom;

    u32 msflag;
    u32 smflag;
};