#include <core/gif/gif.h>
#include <core/system.h>

GIF::GIF(System* system) : system(system) {

}

void GIF::Reset() {
    ctrl = 0;
    std::queue<u128> empty_fifo;
    fifo.swap(empty_fifo);
}

void GIF::SystemReset() {
    log_warn("[GIF] reset gif state");
}

u32 GIF::ReadStat() {
    log_warn("[GIF] read stat %08x", stat);

    return stat;
}

void GIF::WriteCTRL(u8 data) {
    log_warn("[GIF] write ctrl %02x", data);

    if (data & 0x1) {
        SystemReset();
    }

    ctrl = data & 0x9;
}

void GIF::WriteFIFO(u128 data) {
    log_warn("[GIF] write to fifo %016lx%016lx", data.i.hi, data.i.lo);
    fifo.push(data);
}