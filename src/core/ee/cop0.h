#pragma once

#include <common/types.h>
#include <common/log.h>

class EECOP0 {
public:
    void Reset();

    u32 GetReg(int reg);
    void SetReg(int reg, u32 data);

    void CountUp();

    u32 cpr[32];
};