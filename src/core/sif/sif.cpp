#include <core/sif/sif.h>

void SIF::Reset() {
    control = 0;
    bd6 = 0;
    mscom = 0;
    msflag = 0;
    smflag = 0;
    smcom = 0;

    std::queue<u32> sif0_empty;
    std::queue<u32> sif1_empty;
    
    sif0_fifo.swap(sif0_empty);
    sif1_fifo.swap(sif1_empty);
}

void SIF::WriteEEControl(u32 data) {
    if (!(data & 0x100)) {
        control &= ~0x100;
    } else {
        control |= 0x100;
    }
}

void SIF::WriteIOPControl(u32 data) {
    // not sure how this works tbh. figure out later
    u8 value = data & 0xF0;
    if (data & 0xA0) {
        control &= ~0xF000;
        control |= 0x2000;
    }

    if (control & value) {
        control &= ~value;
    } else {
        control |= value;
    }
}

void SIF::WriteBD6(u32 data) {
    // common::Log("[SIF] write bd6 %08x", data);
    bd6 = data;
}

void SIF::WriteMSCOM(u32 data) {
    // common::Log("[SIF] write mscom %08x", data);
    mscom = data;
}

void SIF::WriteSMCOM(u32 data) {
    smcom = data;
}

void SIF::SetMSFLAG(u32 data) {
    // common::Log("[SIF] write msflag %08x", data);
    msflag |= data;
}

void SIF::SetSMFLAG(u32 data) {
    smflag |= data;
}

void SIF::ResetMSFLAG(u32 data) {
    msflag &= ~data;
}

void SIF::ResetSMFLAG(u32 data) {
    smflag &= ~data;
}

u32 SIF::ReadMSFLAG() {
    // common::Log("[SIF] read msflag %08x", msflag);
    return msflag;
}

u32 SIF::ReadMSCOM() {
    // common::Log("[SIF] read mscom %08x", mscom);
    return mscom;
}

u32 SIF::ReadSMFLAG() {
    // common::Log("[SIF] read smflag %08x", smflag);
    return smflag;
}

u32 SIF::ReadSMCOM() {
    // common::Log("[SIF] read smcom %08x", smcom);
    return smcom;
}

u32 SIF::ReadControl() {
    return control;
}

void SIF::write_sif0_fifo(u32 data) {
    sif0_fifo.push(data);
}

void SIF::write_sif1_fifo(u128 data) {
    for (int i = 0; i < 4; i++) {
        sif1_fifo.push(data.uw[i]);
    }
}

u32 SIF::ReadSIF0FIFO() {
    u32 data = sif0_fifo.front();
    sif0_fifo.pop();

    return data;
}

u32 SIF::ReadSIF1FIFO() {
    u32 data = sif1_fifo.front();
    sif1_fifo.pop();

    return data;
}

int SIF::GetSIF0FIFOSize() {
    return sif0_fifo.size();
}

int SIF::GetSIF1FIFOSize() {
    return sif1_fifo.size();
}