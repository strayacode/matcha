#include <cassert>
#include "common/log.h"
#include "core/gs/context.h"
#include "core/system.h"

namespace gs {

Context::Context(System& system) : system(system) {}

void Context::Reset() {
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
    frame.fill(0);
    xyoffset.fill(0);
    scissor.fill(0);
    rgbaq.data = 0;
    xyzf2 = 0;
    xyz2 = 0;
    bitbltbuf = 0;
    trxpos = 0;
    trxreg.data = 0;
    trxdir = 0;
    prmodecont = 0;
    prmode = 0;
    fog = 0;
    st = 0;
    uv = 0;
    scanmsk = 0;
    tex0.fill(0);
    clamp.fill(0);
    xyzf3 = 0;
    xyz3 = 0;
    tex1.fill(0);
    tex2.fill(0);
    texclut = 0;
    miptbp1.fill(0);
    miptbp2.fill(0);
    texa = 0;
    fogcol = 0;
    texflush = 0;
    alpha.fill(0);
    test.fill(0);
    pabe = 0;
    dimx = 0;
    dthe = 0;
    colclamp = 0;
    fba.fill(0);
    zbuf.fill(0);
}

void Context::SystemReset() {
    common::Log("[gs::Context] system reset");
}

u32 Context::ReadRegisterPrivileged(u32 addr) {
    switch (addr) {
    case 0x12001000:
        return csr;
    default:
        common::Error("[gs::Context] handle privileged read %08x", addr);
    }

    return 0;
}

void Context::WriteRegisterPrivileged(u32 addr, u32 value) {
    switch (addr) {
    case 0x12000000:
        pmode = value;
        break;
    case 0x12000004:
        break;
    case 0x12000010:
        smode1 = (smode1 & ~0xFFFFFFFF) | value;
        break;
    case 0x12000014:
        smode1 = ((u64)value << 32) | (smode1 & 0xFFFFFFFF);
        break;
    case 0x12000020:
        smode2 = (smode2 & ~0xFFFFFFFF) | value;
        break;
    case 0x12000024:
        smode2 = ((u64)value << 32) | (smode2 & 0xFFFFFFFF);
        break;
    case 0x12000030:
        srfsh = (srfsh & ~0xFFFFFFFF) | value;
        break;
    case 0x12000034:
        srfsh = ((u64)value << 32) | (srfsh & 0xFFFFFFFF);
        break;
    case 0x12000040:
        synch1 = (synch1 & ~0xFFFFFFFF) | value;
        break;
    case 0x12000044:
        synch1 = ((u64)value << 32) | (synch1 & 0xFFFFFFFF);
        break;
    case 0x12000050:
        synch2 = (synch2 & ~0xFFFFFFFF) | value;
        break;
    case 0x12000054:
        synch2 = ((u64)value << 32) | (synch2 & 0xFFFFFFFF);
        break;
    case 0x12000060:
        syncv = (syncv & ~0xFFFFFFFF) | value;
        break;
    case 0x12000064:
        syncv = ((u64)value << 32) | (syncv & 0xFFFFFFFF);
        break;
    case 0x12000090:
        dispfb2 = (dispfb2 & ~0xFFFFFFFF) | value;
        break;
    case 0x12000094:
        dispfb2 = ((u64)value << 32) | (dispfb2 & 0xFFFFFFFF);
        break;
    case 0x120000A0:
        display2 = (display2 & ~0xFFFFFFFF) | value;
        break;
    case 0x120000A4:
        display2 = ((u64)value << 32) | (display2 & 0xFFFFFFFF);
        break;
    case 0x120000E0:
        bgcolour = value;
        break;
    case 0x120000E4:
        break;
    case 0x12001000:
        if (value & 0x200) {
            SystemReset();
        }

        csr = value;
        break;
    case 0x12001004:
        break;
    case 0x12001010:
        imr = value;
        break;
    case 0x12001014:
        break;
    default:
        common::Error("[gs::Context] handle privileged write %08x = %08x", addr, value);
    }
}

void Context::WriteRegister(u32 addr, u64 value) {
    switch (addr) {
    case 0x00:
        prim = value;
        break;
    case 0x01:
        rgbaq.data = value;
        break;
    case 0x02:
        st = value;
        break;
    case 0x03:
        uv = value;
        break;
    case 0x04:
        xyzf2 = value;
        break;
    case 0x05:
        xyz2 = value;
        break;
    case 0x06:
        tex0[0] = value;
        break;
    case 0x07:
        tex0[1] = value;
        break;
    case 0x08:
        clamp[0] = value;
        break;
    case 0x09:
        clamp[1] = value;
        break;
    case 0x0a:
        fog = value;
        break;
    case 0x0c:
        xyzf3 = value;
        break;
    case 0x0d:
        xyz3 = value;
        break;
    case 0x14:
        tex1[0] = value;
        break;
    case 0x15:
        tex1[1] = value;
        break;
    case 0x16:
        tex2[0] = value;
        break;
    case 0x17:
        tex2[1] = value;
        break;
    case 0x18:
        xyoffset[0] = value;
        break;
    case 0x19:
        xyoffset[1] = value;
        break;
    case 0x1a:
        prmodecont = value;
        break;
    case 0x1b:
        prmode = value;
        break;
    case 0x1c:
        texclut = value;
        break;
    case 0x22:
        scanmsk = value;
        break;
    case 0x34:
        miptbp1[0] = value;
        break;
    case 0x35:
        miptbp1[1] = value;
        break;
    case 0x36:
        miptbp2[0] = value;
        break;
    case 0x37:
        miptbp2[1] = value;
        break;
    case 0x3b:
        texa = value;
        break;
    case 0x3d:
        fogcol = value;
        break;
    case 0x3f:
        texflush = value;
        break;
    case 0x40:
        scissor[0] = value;
        break;
    case 0x41:
        scissor[1] = value;
        break;
    case 0x42:
        alpha[0] = value;
        break;
    case 0x43:
        alpha[1] = value;
        break;
    case 0x44:
        dimx = value;
        break;
    case 0x45:
        dthe = value;
        break;
    case 0x46:
        colclamp = value;
        break;
    case 0x47:
        test[0] = value;
        break;
    case 0x48:
        test[1] = value;
        break;
    case 0x49:
        pabe = value;
        break;
    case 0x4a:
        fba[0] = value;
        break;
    case 0x4b:
        fba[1] = value;
        break;
    case 0x4c:
        frame[0] = value;
        break;
    case 0x4d:
        frame[1] = value;
        break;
    case 0x4e:
        zbuf[0] = value;
        break;
    case 0x4f:
        zbuf[1] = value;
        break;
    case 0x50:
        bitbltbuf = value;
        break;
    case 0x51:
        trxpos = value;
        break;
    case 0x52:
        trxreg.data = value;
        break;
    case 0x53:
        trxdir = value;
        break;
    case 0x54:
        WriteHWReg(value);
        break;
    default:
        common::Error("[gs::Context] handle write %08x = %016lx", addr, value);
    }
}

void Context::WriteHWReg(u64 value) {
    assert(trxdir == 0);
    common::Log("[gs::Context] hwreg write %016llx direction %d width %d height %d", value, trxdir, trxreg.width, trxreg.height);
}

} // namespace gs