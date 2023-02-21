#include <core/vif/vif.h>

void VIF::Reset() {
    fbrst = 0;
    stat = 0;
    mark = 0;
    err = 0;
}

void VIF::SystemReset() {
    common::warn("[VIF] system reset");
}

void VIF::WriteStat(u32 data) {
    if (data) {
        common::error("handle");
    }

    common::warn("[VIF] write stat %08x", data);
    stat = data;
}

void VIF::WriteFBRST(u8 data) {
    common::warn("[VIF] write fbrst %02x", data);
    if (data & 0x1) {
        SystemReset();
    }

    fbrst = data;
}

void VIF::WriteMark(u16 data) {
    common::warn("[VIF] write mark %04x", data);
    stat &= ~(1 << 6);
    mark = data;
}

void VIF::WriteERR(u8 data) {
    common::warn("[VIF] write err %02x", data);
    err = data;
}