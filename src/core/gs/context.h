#pragma once

#include <array>
#include "common/types.h"

struct System;

namespace gs {

class Context {
public:
    Context(System& system);

    u32 ReadRegisterPrivileged(u32 addr);
    void WriteRegisterPrivileged(u32 addr, u32 value);
    void WriteRegister(u32 addr, u64 value);

    void Reset();
    void SystemReset();

    u32 csr;

    // these registers seem to be undocumented
    u64 smode1;
    u64 synch1;
    u64 synch2;
    u64 syncv;
    u64 srfsh;
    u32 imr;

    u8 smode2;
    u32 pmode;
    u64 dispfb2;
    u64 display2;
    u32 bgcolour;
    u32 prim;
    std::array<u64, 2> frame;
    std::array<u64, 2> xyoffset;
    std::array<u64, 2> scissor;
    u64 rgbaq;
    u64 xyzf2;
    u64 xyz2;
    u64 bitbltbuf;
    u64 trxpos;
    u64 trxreg;
    u8 trxdir;
    u64 prmodecont;
    u64 prmode;
    u64 fog;
    u64 st;
    u64 uv;
    u64 scanmsk;
    std::array<u64, 2> tex0;
    std::array<u64, 2> clamp;
    u64 xyzf3;
    u64 xyz3;
    std::array<u64, 2> tex1;
    std::array<u64, 2> tex2;
    u64 texclut;
    std::array<u64, 2> miptbp1;
    std::array<u64, 2> miptbp2;
    u64 texa;
    u64 fogcol;
    u64 texflush;
    std::array<u64, 2> alpha;
    std::array<u64, 2> test;
    u64 pabe;
    u64 dimx;
    u64 dthe;
    u64 colclamp;
    std::array<u64, 2> fba;
    std::array<u64, 2> zbuf;

private:
    System& system;
};

} // namespace gs