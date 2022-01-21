#pragma once

#include "common/types.h"

class SPU {
public:
    void Reset();

    u32 ReadRegister(u32 addr);
    void WriteRegister(u32 addr, u32 data);
    void RequestInterrupt();

private:
    u16 master_volume_left = 0;
    u16 status = 0;
};