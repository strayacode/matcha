#pragma once

#include <common/types.h>
#include <common/log.h>

class IOPCOP0 {
public:
    void Reset();

    u32 GetReg(int reg);
    void SetReg(int reg, u32 data);

    u32 gpr[32];
};