#include <cassert>
#include "common/log.h"
#include "core/gs/context.h"
#include "core/system.h"

namespace gs {

Context::Context(System& system) : system(system) {}

void Context::Reset() {
    csr.data = 0;

    // vertex queue is initially empty
    csr.fifo = 1;

    smode1 = 0;
    synch1 = 0;
    synch2 = 0;
    syncv = 0;
    smode2 = 0;
    srfsh = 0;
    imr = 0;
    pmode.data = 0;
    dispfb2.data = 0;
    display2.data = 0;
    bgcolour = 0;
    prim.data = 0;
    frame.fill(0);
    xyoffset.fill(0);
    scissor.fill(0);
    rgbaq.data = 0;
    bitbltbuf.data = 0;
    trxpos.data = 0;
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

    pixels_transferred = 0;
    
    for (int i = 0; i < 512; i++) {
        vram[i].Reset();
    }

    for (int y = 0; y < 480; y++) {
        for (int x = 0; x < 640; x++) {
            framebuffer[y][x] = 0;
        }
    }

    current_vertex.x = 0;
    current_vertex.y = 0;
    current_vertex.z = 0;
    current_vertex.r = 0;
    current_vertex.g = 0;
    current_vertex.b = 0;
    current_vertex.a = 0;
    current_vertex.q = 0.0f;
    current_vertex.fog = 0;

    vertex_queue.Reset();
}

void Context::SystemReset() {
    common::Log("[gs::Context] system reset");
}

void Context::RenderCRTC() {
    int width = GetCRTCWidth();
    int height = GetCRTCHeight();
    int dx = 0;
    int dy = 0;
    if (pmode.en1 && pmode.en2) {
        common::Error("[gs::Context] read circuit 1 and read circuit 2 enabled");
    } else if (pmode.en1) {
        common::Error("[gs::Context] read circuit 1 enabled");
    } else if (pmode.en2) {
        dx = display2.dx / (display2.magh + 1);
        dy = display2.dy / (display2.magv + 1);
    }

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            assert(pmode.en2 && !pmode.en1);
            int coord_x = x + dispfb2.dbx;
            int coord_y = y + dispfb2.dby;
            u32 pixel = GetCRTCPixel(dispfb2.fbp * 2048 * 4, coord_x, coord_y, dispfb2.fbw * 64, static_cast<PixelFormat>(dispfb2.psm));
            framebuffer[coord_y][coord_x] = pixel;
        }
    }
}

Framebuffer Context::GetFramebuffer() {
    return {
        .data = reinterpret_cast<u8*>(&framebuffer),
        .width = GetCRTCWidth(),
        .height = GetCRTCHeight()
    };
}

u32 Context::ReadRegisterPrivileged(u32 addr) {
    switch (addr) {
    case 0x12001000:
        return csr.data;
    case 0x12001004:
        return 0;
    default:
        common::Error("[gs::Context] handle privileged read %08x", addr);
    }

    return 0;
}

void Context::WriteRegisterPrivileged(u32 addr, u32 value) {
    switch (addr) {
    case 0x12000000:
        pmode.data = value;
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
        dispfb2.data = (dispfb2.data & ~0xFFFFFFFF) | value;
        break;
    case 0x12000094:
        dispfb2.data = ((u64)value << 32) | (dispfb2.data & 0xFFFFFFFF);
        break;
    case 0x120000A0:
        display2.data = (display2.data & ~0xFFFFFFFF) | value;
        break;
    case 0x120000A4:
        display2.data = ((u64)value << 32) | (display2.data & 0xFFFFFFFF);
        break;
    case 0x120000E0:
        bgcolour = value;
        break;
    case 0x120000E4:
        break;
    case 0x12001000:
        csr.data = value;

        if (csr.reset) {
            SystemReset();
        }

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
        prim.data = value;
        common::Log("[gs::Context] primitive type is now %d", prim.prim);
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
        common::Log("[gs::Context] xyzf2 write %016llx", value);
        current_vertex.x = value & 0xffff;
        current_vertex.y = (value >> 16) & 0xffff;
        current_vertex.z = (value >> 32) & 0xffffff;
        current_vertex.fog = (value >> 56) & 0xff;
        VertexKick();
        DrawingKick();
        break;
    case 0x05:
        common::Log("[gs::Context] xyz2 write %016llx", value);
        current_vertex.x = value & 0xffff;
        current_vertex.y = (value >> 16) & 0xffff;
        current_vertex.z = (value >> 32) & 0xffffffff;
        VertexKick();
        DrawingKick();
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
        common::Log("[gs::Context] xyzf3 write %016llx", value);
        current_vertex.x = value & 0xffff;
        current_vertex.y = (value >> 16) & 0xffff;
        current_vertex.z = (value >> 32) & 0xffffff;
        current_vertex.fog = (value >> 56) & 0xff;
        VertexKick();
        break;
    case 0x0d:
        common::Log("[gs::Context] xyz3 write %016llx", value);
        current_vertex.x = value & 0xffff;
        current_vertex.y = (value >> 16) & 0xffff;
        current_vertex.z = (value >> 32) & 0xffffffff;
        VertexKick();
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
        common::Log("[gs::Context] prmodecont write %016llx", prmodecont);
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
        bitbltbuf.data = value;
        break;
    case 0x51:
        trxpos.data = value;
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

    // (in bytes)
    u32 dst_base = bitbltbuf.dst_base * 64 * 4;

    // (in pixels)
    u32 dst_width = bitbltbuf.dst_width * 64;

    int pixels_to_transfer = GetPixelsToTransfer(static_cast<PixelFormat>(bitbltbuf.dst_format));
    for (int i = 0; i < pixels_to_transfer; i++) {
        int x = (pixels_transferred % trxreg.width) + trxpos.dst_x;
        int y = (pixels_transferred / trxreg.width) + trxpos.dst_y;
        // common::Log("[gs::Context] x %d y %d", x, y);

        if (x >= 2048) {
            common::Error("[gs::Context] handle x >= 2048");
        }

        if (y >= 2048) {
            common::Error("[gs::Context] handle y >= 2048");
        }

        switch (static_cast<PixelFormat>(bitbltbuf.dst_format)) {
        case PixelFormat::PSMCT32:
            WritePSMCT32Pixel(dst_base, x, y, dst_width, (value >> (32 * i)) & 0xffffffff);
            break;
        default:
            common::Error("[gs::Context] handle destination format %d", bitbltbuf.dst_format);
        }

        pixels_transferred++;
    }

    if (pixels_transferred >= (trxreg.width * trxreg.height)) {
        common::Log("[gs::Context] end of gif->vram transfer");
        pixels_transferred = 0;
        trxdir = 3;
    }
}

int Context::GetPixelsToTransfer(PixelFormat format) {
    switch (format) {
    case PixelFormat::PSMCT32:
        return 2;
    default:
        common::Error("[gs::Context] handle destination format %d", static_cast<int>(format));
    }

    return 0;
}

u32 Context::GetCRTCPixel(u32 base, int x, int y, u32 width, PixelFormat format) {
    switch (format) {
    case PixelFormat::PSMCT32:
        return ReadPSMCT32Pixel(base, x, y, width);
    default:
        common::Error("[gs::Context] handle crtc pixel format %d", static_cast<int>(format));
    }

    return 0;
}

int Context::GetCRTCWidth() {
    if (pmode.en1 && pmode.en2) {
        common::Error("[gs::Context] read circuit 1 and read circuit 2 enabled");
    } else if (pmode.en1) {
        common::Error("[gs::Context] read circuit 1 enabled");
    } else if (pmode.en2) {
        return (display2.dw + 1) / (display2.magh + 1);
    }

    return 0;
}

int Context::GetCRTCHeight() {
    if (pmode.en1 && pmode.en2) {
        common::Error("[gs::Context] read circuit 1 and read circuit 2 enabled");
    } else if (pmode.en1) {
        common::Error("[gs::Context] read circuit 1 enabled");
    } else if (pmode.en2) {
        return (display2.dh + 1) / (display2.magv + 1);
    }

    return 0;
}

u32 Context::ReadPSMCT32Pixel(u32 base, int x, int y, u32 width) {
    // base is a byte address, and pages are stored as units of 8192 bytes sequentially,
    // so / 8192 will give us the current page
    int page = base / 8192;
    int width_in_pages = width / 64;

    // add the horizontal increment from x to page
    page += (x / 64) % width_in_pages;

    // add the vertical increment from y to page
    page += (y / 32) * width_in_pages;

    return vram[page].ReadPSMCT32Pixel(x, y);
}

void Context::WritePSMCT32Pixel(u32 base, int x, int y, u32 width, u32 value) {
    // base is a byte address, and pages are stored as units of 8192 bytes sequentially,
    // so / 8192 will give us the current page
    int page = base / 8192;
    int width_in_pages = width / 64;

    // add the horizontal increment from x to page
    page += (x / 64) % width_in_pages;

    // add the vertical increment from y to page
    page += (y / 32) * width_in_pages;

    vram[page].WritePSMCT32Pixel(x, y, value);
}

void Context::VertexKick() {
    common::Log("[gs::Context] do vertex kick for primitive %d", prim.prim);
}

void Context::DrawingKick() {
    common::Log("[gs::Context] do drawing kick for primitive %d", prim.prim);
}

} // namespace gs