#include "common/log.h"
#include "core/gif.h"
#include "core/system.h"

GIF::GIF(gs::Context& gs) : gs(gs) {}

void GIF::Reset() {
    ctrl = 0;
    fifo.Reset();

    current_tag.nloop = 0;
    current_tag.eop = false;
    current_tag.prim = false;
    current_tag.prim_data = 0;
    current_tag.format = 0;
    current_tag.nregs = 0;
    current_tag.reglist = 0;
    current_tag.reglist_offset = 0;
    current_tag.transfers_left = 0;
}

void GIF::SystemReset() {
    common::Log("[GIF] reset gif state");
}

void GIF::Run(int cycles) {
    while (!fifo.Empty() && cycles--) {
        if (current_tag.transfers_left) {
            ProcessTag();
        } else {
            StartTransfer();
        }
    }
}

u32 GIF::ReadRegister(u32 addr) {
    switch (addr) {
    case 0x10003020:
        common::Log("[GIF] read stat %08x", stat);
        return stat;
    default:
        common::Error("[GIF] handle read %08x", addr);
    }

    return 0;
}

void GIF::WriteRegister(u32 addr, u32 value) {
    if (addr >= 0x10006000 && addr < 0x10006010) {
        WriteFIFO(value);
        return;
    }

    switch (addr) {
    case 0x10003000:
        common::Log("[GIF] write ctrl %02x", value);

        if (value & 0x1) {
            SystemReset();
        }

        ctrl = value & 0x9;
        break;
    default:
        common::Error("[GIF] handle write %08x = %08x", addr, value);
    }
}

void GIF::WriteFIFO(u32 value) {
    fifo.Push<u32>(value);
    // common::Log("[GIF] push to fifo %08x", value);
    if (fifo.GetLength() == fifo.GetSize()) {
        common::Error("[GIF] no more space left in fifo");
    }
}

void GIF::SendPath3(u128 value) {
    // common::Log("[GIF] send path3 %016lx%016lx format %d", value.hi, value.lo);
    fifo.Push<u128>(value);
}

void GIF::ProcessPacked(u128 data) {
    u8 reg = (current_tag.reglist >> (current_tag.reglist_offset * 4)) & 0xf;

    switch (reg) {
    case 0x0:
        gs.WriteRegister(0x00, data.uw[0] & 0x7ff);
        break;
    case 0x1:
        common::Log("[GIF] write rgbaq");
        break;
    case 0xa:
        common::Log("[GIF] write fog");
        break;
    case 0xe:
        gs.WriteRegister(data.hi & 0xFF, data.lo);
        break;
    default:
        common::Error("[GIF] handle register %02x", reg);
    }

    current_tag.reglist_offset++;

    if (current_tag.reglist_offset == current_tag.nregs) {
        current_tag.reglist_offset = 0;
    }
}

void GIF::ProcessImage(u128 data) {
    gs.WriteHWReg(data.lo);
    gs.WriteHWReg(data.hi);
}

void GIF::StartTransfer() {
    // read a new giftag from the fifo
    u128 data = fifo.Pop<u128>();
    current_tag.nloop = data.lo & 0x7fff;
    current_tag.eop = (data.lo >> 15) & 0x1;
    current_tag.prim = (data.lo >> 46) & 0x1;
    current_tag.prim_data = (data.lo >> 47) & 0x7ff;
    current_tag.format = (data.lo >> 58) & 0x3;
    current_tag.nregs = (data.lo >> 60) & 0xf;
    current_tag.reglist = data.hi;
    current_tag.reglist_offset = 0;

    // common::Log("[GIF] start transfer %016lx%016lx format %d", data.hi, data.lo, current_tag.format);

    if (!current_tag.nregs) {
        current_tag.nregs = 16;
    }

    if (current_tag.prim) {
        gs.prim = current_tag.prim_data;
    }

    gs.rgbaq.q = 1.0f;

    switch (current_tag.format) {
    case 0:
        current_tag.transfers_left = current_tag.nloop * current_tag.nregs;
        break;
    case 2:
        current_tag.transfers_left = current_tag.nloop;
        break;
    default:
        common::Error("[GIF] handle giftag format start of transfer %d", current_tag.format);
    }
}

void GIF::ProcessTag() {
    u128 data = fifo.Pop<u128>();
    // common::Log("[GIF] receive giftag in transfer %016lx%016lx", data.hi, data.lo);
    switch (current_tag.format) {
    case 0:
        ProcessPacked(data);
        break;
    case 2:
        ProcessImage(data);
        break;
    default:
        common::Error("[GIF] handle giftag in transfer format %d", current_tag.format);
    }

    current_tag.transfers_left--;
}