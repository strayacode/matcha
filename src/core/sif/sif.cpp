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

void SIF::WriteBD6(u32 data) {
    log_warn("[SIF] write bd6 %08x", data);
    bd6 = data;
}

void SIF::WriteMSCOM(u32 data) {
    log_warn("[SIF] write mscom %08x", data);
    mscom = data;
}

void SIF::WriteMSFLAG(u32 data) {
    log_warn("[SIF] write msflag %08x", data);
    msflag = data;
}

u32 SIF::ReadMSFLAG() {
    log_warn("[SIF] read msflag %08x", msflag);
    return msflag;
}

u32 SIF::ReadMSCOM() {
    log_warn("[SIF] read mscom %08x", mscom);
    return mscom;
}

u32 SIF::ReadSMFLAG() {
    log_warn("[SIF] read smflag %08x", 0x10000);
    // fake the iop response for fastbooting isos and elfs
    return 0x10000;
}

u32 SIF::ReadSMCOM() {
    log_warn("[SIF] read smcom %08x", smcom);
    return smcom;
}