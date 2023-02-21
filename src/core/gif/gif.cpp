#include "common/log.h"
#include "core/gif/gif.h"
#include "core/system.h"

GIF::GIF(System& system) : system(system) {}

void GIF::Reset() {
    ctrl = 0;
    std::queue<u128> empty_fifo;
    fifo.swap(empty_fifo);

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
    common::Warn("[GIF] reset gif state");
}

u32 GIF::ReadStat() {
    common::Warn("[GIF] read stat %08x", stat);

    return stat;
}

void GIF::WriteCTRL(u8 data) {
    common::Warn("[GIF] write ctrl %02x", data);

    if (data & 0x1) {
        SystemReset();
    }

    ctrl = data & 0x9;
}

void GIF::WriteFIFO(u128 data) {
    // common::Warn("[GIF] write to fifo %016lx%016lx", data.i.hi, data.i.lo);
    fifo.push(data);
}

void GIF::SendPath3(u128 data) {
    if (!current_tag.transfers_left) {
        // read a new giftag
        current_tag.nloop = data.ud[0] & 0x7FFF;
        current_tag.eop = (data.ud[0] >> 15) & 0x1;
        current_tag.prim = (data.ud[0] >> 46) & 0x1;
        current_tag.prim_data = (data.ud[0] >> 47) & 0x7FF;
        current_tag.format = (data.ud[0] >> 58) & 0x3;
        current_tag.nregs = (data.ud[0] >> 60) & 0xF;
        current_tag.reglist = data.ud[1];
        current_tag.reglist_offset = 0;

        common::Log("[GIF] receive giftag %016lx%016lx format %d", data.ud[1], data.ud[0], current_tag.format);

        if (!current_tag.nregs) {
            current_tag.nregs = 16;
        }

        // TODO: initialise the Q register to 1.0f

        switch (current_tag.format) {
        case 0:
            current_tag.transfers_left = current_tag.nloop * current_tag.nregs;
            break;
        default:
            common::Error("[GIF] handle GIFTag format %d", current_tag.format);
        }
    } else {
        switch (current_tag.format) {
        case 0:
            ProcessPacked(data);
            break;
        default:
            common::Error("[GIF] handle GIFTag format %d", current_tag.format);
        }

        current_tag.transfers_left--;
    }
}

void GIF::ProcessPacked(u128 data) {
    u8 reg = (current_tag.reglist >> (current_tag.reglist_offset * 4)) & 0xF;

    switch (reg) {
    case 0x00:
        system.gs.WriteRegister(0x00, data.uw[0] & 0x7FF);
        break;
    case 0x0E:
        system.gs.WriteRegister(data.ud[1] & 0xFF, data.ud[0]);
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

}