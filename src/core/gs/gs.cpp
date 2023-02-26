#include "common/log.h"
#include "core/gs/gs.h"
#include "core/system.h"

GS::GS(System* system) : system(system) {

}

void GS::Reset() {
    csr = 0;
    smode1 = 0;
    synch1 = 0;
    synch2 = 0;
    syncv = 0;
    smode2 = 0;
    srfsh = 0;
    imr = 0;
    pmode = 0;
    dispfb2 = 0;
    display2 = 0;
    bgcolour = 0;
    prim = 0;
    frame1 = 0;
    xyoffset1 = 0;
    scissor1 = 0;
    rgbaq = 0;
    xyz2 = 0;
    bitbltbuf = 0;
    trxpos = 0;
    trxreg = 0;
    trxdir = 0;
}

void GS::SystemReset() {
    common::Warn("[GS] system reset");
}

u32 GS::ReadRegisterPrivileged(u32 addr) {
    switch (addr) {
    case 0x12001000:
        return csr;
    default:
        common::Error("[GS] handle privileged read %08x", addr);
    }

    return 0;
}

void GS::WriteRegisterPrivileged(u32 addr, u32 data) {
    switch (addr) {
    case 0x12000000:
        pmode = data;
        break;
    case 0x12000004:
        break;
    case 0x12000010:
        smode1 = (smode1 & ~0xFFFFFFFF) | data;
        break;
    case 0x12000014:
        smode1 = ((u64)data << 32) | (smode1 & 0xFFFFFFFF);
        break;
    case 0x12000020:
        smode2 = (smode2 & ~0xFFFFFFFF) | data;
        break;
    case 0x12000024:
        smode2 = ((u64)data << 32) | (smode2 & 0xFFFFFFFF);
        break;
    case 0x12000030:
        srfsh = (srfsh & ~0xFFFFFFFF) | data;
        break;
    case 0x12000034:
        srfsh = ((u64)data << 32) | (srfsh & 0xFFFFFFFF);
        break;
    case 0x12000040:
        synch1 = (synch1 & ~0xFFFFFFFF) | data;
        break;
    case 0x12000044:
        synch1 = ((u64)data << 32) | (synch1 & 0xFFFFFFFF);
        break;
    case 0x12000050:
        synch2 = (synch2 & ~0xFFFFFFFF) | data;
        break;
    case 0x12000054:
        synch2 = ((u64)data << 32) | (synch2 & 0xFFFFFFFF);
        break;
    case 0x12000060:
        syncv = (syncv & ~0xFFFFFFFF) | data;
        break;
    case 0x12000064:
        syncv = ((u64)data << 32) | (syncv & 0xFFFFFFFF);
        break;
    case 0x12000090:
        dispfb2 = (dispfb2 & ~0xFFFFFFFF) | data;
        break;
    case 0x12000094:
        dispfb2 = ((u64)data << 32) | (dispfb2 & 0xFFFFFFFF);
        break;
    case 0x120000A0:
        display2 = (display2 & ~0xFFFFFFFF) | data;
        break;
    case 0x120000A4:
        display2 = ((u64)data << 32) | (display2 & 0xFFFFFFFF);
        break;
    case 0x120000E0:
        bgcolour = data;
        break;
    case 0x120000E4:
        break;
    case 0x12001000:
        if (data & 0x200) {
            SystemReset();
        }

        csr = data;
        break;
    case 0x12001004:
        break;
    case 0x12001010:
        imr = data;
        break;
    case 0x12001014:
        break;
    default:
        common::Error("[GS] handle privileged write %08x = %08x", addr, data);
    }
}

void GS::WriteRegister(u32 addr, u64 data) {
    switch (addr) {
    case 0x00:
        prim = data;
        break;
    case 0x01:
        rgbaq = data;
        break;
    case 0x05:
        xyz2 = data;
        break;
    case 0x18:
        xyoffset1 = data;
        break;
    case 0x40:
        scissor1 = data;
        break;
    case 0x4C:
        frame1 = data;
        break;
    case 0x50:
        bitbltbuf = data;
        break;
    case 0x51:
        trxpos = data;
        break;
    case 0x52:
        trxreg = data;
        break;
    case 0x53:
        trxdir = data;
        break;
    default:
        common::Error("[GS] handle write %08x = %016lx", addr, data);
    }
}
