#pragma once

#include <common/types.h>
#include <common/log.h>

union FPR {
    f32 f;
    u32 u;
    s32 s;
};

class EECOP1 {
public:
    void Reset();

    u32 GetReg(int reg);
    void SetReg(int reg, u32 data);

    FPR fpr[32];
};