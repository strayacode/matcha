#pragma once

#include "common/types.h"

struct System;

class GS {
public:
    GS(System& system);

    u64 ReadRegisterPrivileged(u32 addr);
    void WriteRegisterPrivileged(u32 addr, u64 value);
    void WriteRegister(u32 addr, u64 value);

    void Reset();
    void SystemReset();

private:
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
    u64 frame1;
    u64 xyoffset1;
    u64 scissor1;
    u64 rgbaq;
    u64 xyz2;
    u64 bitbltbuf;
    u64 trxpos;
    u64 trxreg;
    u8 trxdir;

    System& system;
};