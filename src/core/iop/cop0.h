#pragma once

#include <array>
#include "common/types.h"

namespace iop {

struct COP0 {
    void Reset();

    u32 GetReg(int reg);
    void SetReg(int reg, u32 value);

    union Status {
        struct {
            bool iec : 1;
            bool kuc : 1;
            bool iep : 1;
            bool kup : 1;
            bool ieo : 1;
            bool kuo : 1;
            u32 : 2;
            u32 im : 8;
            bool isc : 1;
            bool swc : 1;
            bool pz : 1;
            bool cm : 1;
            bool pe : 1;
            bool ts : 1;
            bool bev : 1;
            u32 : 2;
            bool re : 1;
            u32 : 2;
            bool cu0 : 1;
            bool cu1 : 1;
            bool cu2 : 1;
            bool cu3 : 1;
        };

        u32 data;
    };

    union Cause {
        struct {
            u32 : 2;
            u8 excode : 5;
            u32 : 1;
            u32 ip : 8;
            u32 : 12;
            u32 ce : 2;
            u32 : 1;
            bool bd : 1;
        };

        u32 data;
    };

    Status status;
    Cause cause;
    u32 epc;
    u32 prid;
};

} // namespace iop