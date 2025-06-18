#pragma once

#include "common/types.h"

typedef struct {
    u16 master_volume_left;
    u16 status;
} spu_t;

spu_t* spu_init(void);

u32 spu_read(spu_t* spu, u32 addr);
void spu_write(spu_t* spu, u32 addr, u32 data);
void spu_request_interrupt(spu_t* spu);