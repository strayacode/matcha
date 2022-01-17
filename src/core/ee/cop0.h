#pragma once

#include <array>
#include "common/types.h"
#include "common/log.h"

class EECOP0 {
public:
    void Reset();

    u32 GetReg(int reg);
    void SetReg(int reg, u32 data);

    void CountUp();

    std::array<u32, 32> gpr;
};