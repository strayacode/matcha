#include "common/log.h"
#include "core/spu/spu.h"

void SPU::Reset() {
    master_volume_left = 0;
    status = 0;
}

u32 SPU::ReadRegister(u32 addr) {
    u32 return_value = 0;

    switch (addr) {
    case 0x1F900744:
        // status
        return_value = status;
        status &= ~0x80;
        break;
    default:
        common::Log("[SPU] handle read %08x", addr);
    }

    return return_value;
}

void SPU::WriteRegister(u32 addr, u32 data) {
    switch (addr) {
    case 0x1F900744:
        // status
        break;
    default:
        common::Log("[SPU] handle write %08x = %08x", addr, data);
    }
}

void SPU::RequestInterrupt() {
    status |= 0x80;
}