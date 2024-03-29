#pragma once

#include <array>
#include <memory>
#include "common/queue.h"
#include "common/types.h"
#include "core/gs/page.h"

struct System;

namespace gs {

// vram notes:
// vram is a 4mb linear array, which consists of pages, blocks and columns
// vram contains 512 pages
// 1 page = 32 blocks
// 1 block = 4 columns
// 1 column contains pixels

// crtc notes:
// the crtc is responsible for outputting the final image to the tv,
// which it does by reading vram and can be configured through various registers
// process:
// read 2 rectangles from vram called output circuit 1 and 2
// these then get merged together into the merge circuit which gets outputted to your screen
// dispfb register (for output circuit 1 and 2):
// fbp: is used to specify the base in vram
// fbw: specifies the framebuffer width
// dbx: x offset from the frame
// dby: y offset from the frame
// display register (for output circuit 1 and 2):
// dx: x offset on screen
// dy: y offset on screen
// dw + 1: width on screen
// dy + 1: height on screen

// vertex kick is done when writes to specific gs registers are done. this will cause a vertex to be
// added to the vertex queue

// drawing kick is done when writes to specific gs registers are done. this will cause all
// the vertices in the vertex queue to be combined together to draw a primitive

struct Framebuffer {
    u8* data;
    int width;
    int height;
};

class Context {
public:
    Context(System& system);

    u32 ReadRegisterPrivileged(u32 addr);
    void WriteRegisterPrivileged(u32 addr, u32 value);
    void WriteRegister(u32 addr, u64 value);
    void WriteHWReg(u64 value);

    void RenderCRTC();

    void Reset();
    void SystemReset();

    Framebuffer GetFramebuffer();

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

    union PMODE {
        struct {
            bool en1 : 1;
            bool en2 : 1;
            u32 crtmd : 3;
            bool mmod : 1;
            bool amod : 1;
            bool slbg : 1;
            u8 alp : 8;
            u32 : 16;
        };

        u32 data;
    };

    union DISPLAY {
        struct {
            u32 dx : 12;
            u32 dy : 11;
            u32 magh : 4;
            u32 magv : 2;
            u32 : 3;
            u32 dw : 12;
            u32 dh : 11;
            u32 : 9;
        };

        u64 data;
    };

    union DISPFB {
        struct {
            u32 fbp : 9;
            u32 fbw : 6;
            u32 psm : 5;
            u32 : 12;
            u32 dbx : 11;
            u32 dby : 11;
            u32 : 10;
        };

        u64 data;
    };

    enum PrimitiveType : int {
        Point = 0,
        Line = 1,
        LineStrip = 2,
        Triangle = 3,
        TriangleStrip = 4,
        TriangleFan = 5,
        Sprite = 6,
    };

    union PRIM {
        struct {
            u32 prim : 3;
            bool iip : 1;
            bool tme : 1;
            bool fge : 1;
            bool abe : 1;
            bool aa1 : 1;
            bool fst : 1;
            bool ctxt : 1;
            bool fix : 1;
            u32 : 21;
        };

        u32 data;
    };

    union CSR {
        struct {
            bool signal : 1;
            bool finish : 1;
            bool hsint : 1;
            bool vsint : 1;
            bool edwint : 1;
            u32 : 3;
            bool flush : 1;
            bool reset : 1;
            u32 : 2;
            bool nfield : 1;
            bool field : 1;
            u32 fifo : 2;
            u32 rev : 8;
            u32 id : 8;
        };

        u32 data;
    };

    CSR csr;

    // these registers seem to be undocumented
    u64 smode1;
    u64 synch1;
    u64 synch2;
    u64 syncv;
    u64 srfsh;
    u32 imr;

    u8 smode2;
    PMODE pmode;
    DISPFB dispfb2;
    DISPLAY display2;
    u32 bgcolour;
    PRIM prim;
    std::array<u64, 2> frame;
    std::array<u64, 2> xyoffset;
    std::array<u64, 2> scissor;
    RGBAQ rgbaq;
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

    struct Vertex {
        // these are fixed point integers,
        // with 12 bits for integer and 4 bits for decimal
        u16 x;
        u16 y;

        u32 z;

        // colour values
        u8 r;
        u8 g;
        u8 b;
        u8 a;
        f32 q;

        u8 fog;
    };

    int GetPixelsToTransfer(PixelFormat format);
    u32 GetCRTCPixel(u32 base, int x, int y, u32 width, PixelFormat format);
    int GetCRTCWidth();
    int GetCRTCHeight();
    u32 ReadPSMCT32Pixel(u32 base, int x, int y, u32 width);
    void WritePSMCT32Pixel(u32 base, int x, int y, u32 width, u32 value);

    void VertexKick();
    void DrawingKick();

    int pixels_transferred;
    
    std::array<Page, 512> vram;
    u32 framebuffer[480][640];
    Vertex current_vertex;
    common::Queue<Vertex, 3> vertex_queue;
    System& system;
};

} // namespace gs