#pragma once

#include <array>
#include <memory>
#include "common/types.h"

struct System;

namespace gs {

// vram notes:
// vram is a 4mb linear array, which consists of pages, blocks and columns
// vram contains 512 pages
// 1 page = 32 blocks
// 1 block = 4 columns
// 1 column contains pixels

class Context {
public:
    Context(System& system);

    u32 ReadRegisterPrivileged(u32 addr);
    void WriteRegisterPrivileged(u32 addr, u32 value);
    void WriteRegister(u32 addr, u64 value);
    void WriteHWReg(u64 value);

    void Reset();
    void SystemReset();

    union RGBAQ {
        struct {
            u8 r;
            u8 g;
            u8 b;
            u8 a;
            f32 q;
        };

        u64 data;
    };

    union TRXREG {
        struct {
            u32 width : 12;
            u32 : 20;
            u32 height : 12;
            u32 : 20;
        };

        u64 data;
    };

    union BITBLTBUF {
        struct {
            u32 src_base : 14;
            u32 : 2;
            u32 src_width : 6;
            u32 : 2;
            u32 src_format : 6;
            u32 : 2;
            u32 dst_base : 14;
            u32 : 2;
            u32 dst_width : 6;
            u32 : 2;
            u32 dst_format : 6;
            u32 : 2;
        };

        u64 data;
    };

    union TRXPOS {
        struct {
            u32 src_x : 11;
            u32 : 5;
            u32 src_y : 11;
            u32 : 5;
            u32 dst_x : 11;
            u32 : 5;
            u32 dst_y : 11;
            u32 order : 2;
            u32 : 3;
        };

        u64 data;
    };

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
    RGBAQ rgbaq;
    u64 xyzf2;
    u64 xyz2;
    BITBLTBUF bitbltbuf;
    TRXPOS trxpos;
    TRXREG trxreg;
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
    enum class PixelFormat : int {
        PSMCT32 = 0x00,
        PSMCT24 = 0x01,
        PSMCT16 = 0x02,
        PSMCT16S = 0x0A,
        PSMCT8 = 0x13,
        PSMCT4 = 0x14,
        PSMCT8H = 0x1b,
        PSMCT4HL = 0x24,
        PSMCT4HH = 0x2c,
        PSMZ32 = 0x30,
        PSMZ24 = 0x31,
        PSMZ16 = 0x32,
        PSMZ16S = 0x3a,
    };

    int GetPixelsToTransfer(PixelFormat format);
    void WritePSMCT32Pixel(u32 base, int x, int y, u32 width, u32 pixel);

    int pixels_transferred;
    
    std::array<Page, 512> vram;

    System& system;
};

} // namespace gs