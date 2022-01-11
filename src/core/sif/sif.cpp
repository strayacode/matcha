#include <core/sif/sif.h>

void SIF::Reset() {
    control = 0;
    bd6 = 0;
    mscom = 0;
    msflag = 0;
    smflag = 0;
    smcom = 0;
}

void SIF::WriteEEControl(u32 data) {
    if (!(data & (0x100))) {
        control &= ~0x100;
    } else {
        control |= 0x100;
    }
}

void SIF::WriteIOPControl(u32 data) {
    // not sure how this works tbh. figure out later
    u8 value = data & 0xF0;

    if (data & 0xA0)
    {
        control &= ~0xF000;
        control |= 0x2000;
    }

    if (control & value)
        control &= ~value;
    else
        control |= value;
}

void SIF::WriteBD6(u32 data) {
    // log_warn("[SIF] write bd6 %08x", data);
    bd6 = data;
}

void SIF::WriteMSCOM(u32 data) {
    // log_warn("[SIF] write mscom %08x", data);
    mscom = data;
}

void SIF::WriteSMCOM(u32 data) {
    smcom = data;
}

void SIF::SetMSFLAG(u32 data) {
    // log_warn("[SIF] write msflag %08x", data);
    msflag |= data;
}

void SIF::SetSMFLAG(u32 data) {
    smflag |= data;
}

void SIF::ResetSMFLAG(u32 data) {
    smflag &= ~data;
}

u32 SIF::ReadMSFLAG() {
    // log_warn("[SIF] read msflag %08x", msflag);
    return msflag;
}

u32 SIF::ReadMSCOM() {
    // log_warn("[SIF] read mscom %08x", mscom);
    return mscom;
}

u32 SIF::ReadSMFLAG() {
    // log_warn("[SIF] read smflag %08x", smflag);
    return smflag;
}

u32 SIF::ReadSMCOM() {
    // log_warn("[SIF] read smcom %08x", smcom);
    return smcom;
}

u32 SIF::ReadControl() {
    return control;
}