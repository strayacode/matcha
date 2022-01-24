#include "common/log.h"
#include "core/gs/gs.h"

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
}

void GS::SystemReset() {
    log_warn("[GS] system reset");
}

u32 GS::ReadRegisterPrivileged(u32 addr) {
    switch (addr) {
    case 0x12001000:
        return csr;
    default:
        log_fatal("[GS] handle privileged read %08x", addr);
    }
}

void GS::WriteCSR(u32 data) {
    log_warn("[GS] write csr %08x", data);
    if (data & 0x200) {
        SystemReset();
    }

    csr = data;
}

void GS::WriteSMODE1(u64 data) {
    log_warn("[GS] write smode1 %016lx", data);
    smode1 = data;
}

void GS::WriteSYNCH1(u64 data) {
    log_warn("[GS] write synch1 %016lx", data);
    synch1 = data;
}

void GS::WriteSYNCH2(u64 data) {
    log_warn("[GS] write synch2 %016lx", data);
    synch2 = data;
}

void GS::WriteSYNCV(u64 data) {
    log_warn("[GS] write syncv %016lx", data);
    syncv = data;
}

void GS::WriteSMODE2(u8 data) {
    log_warn("[GS] write smode2 %02x", data);
    smode2 = data;
}

void GS::WriteSRFSH(u64 data) {
    log_warn("[GS] write srfsh %016lx", data);
    srfsh = data;
}