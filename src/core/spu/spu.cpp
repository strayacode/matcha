#include "common/log.h"
#include "core/spu/spu.h"
#include <stdlib.h>

spu_t* spu_init(void) {
    spu_t* spu = malloc(sizeof(spu_t));
    spu->master_volume_left = 0;
    spu->status = 0;

    return spu;
}

u32 spu_read(spu_t* spu, u32 addr) {
    switch (addr) {
    case 0x1f900744: {
        u32 result = spu->status;
        spu->status &= ~0x80;

        return result;
    }
    default:
        common::Log("[SPU] handle read %08x", addr);
        return 0;
    }
}

void spu_write(spu_t* spu, u32 addr, u32 data) {
    switch (addr) {
    case 0x1f900744:
        // status
        break;
    default:
        common::Log("[SPU] handle write %08x = %08x", addr, data);
    }
}

void spu_request_interrupt(spu_t* spu) {
    spu->status |= 0x80;
}