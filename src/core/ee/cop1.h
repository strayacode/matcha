#pragma once

#include <common/types.h>
#include <common/log.h>

union FPURegister {
    f32 f;
    u32 u;
    s32 s;
};

class EECOP1 {
public:
    void Reset();

    u32 GetReg(int reg);
    void SetReg(int reg, u32 data);
    void SetControlReg(int reg, u32 data);
    f32 AsFloat(u32 value);

    FPURegister fpr[32];
    FPURegister accumulator;
    u32 control[32];
};