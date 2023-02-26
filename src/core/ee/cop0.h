#pragma once

#include <array>
#include "common/types.h"
#include "common/log.h"

namespace ee {

class COP0 {
public:
    void Reset();

    u32 GetReg(int reg);
    void SetReg(int reg, u32 data);

    void CountUp();

    union Cause {
        struct {
            u32 : 2;
            u8 exception : 5;
            u32 : 3;
            bool int0_pending : 1;
            bool int1_pending : 1;
            u32 : 3;
            bool timer_pending : 1;
            u8 error : 3;
            u32 : 9;
            u8 cu : 2;
            bool bd2 : 1;
            bool bd : 1;
        };

        u32 data;
    } cause;

    u32 index;
    std::array<u32, 32> gpr;

private:
    // structure of a tlb entry
    struct Entry {

    };
};

} // namespace ee