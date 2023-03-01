#pragma once

#include "common/types.h"

namespace iop {

struct SIO2 {
    void Reset();

    u32 ReadRegister(u32 addr);
    void WriteRegister(u32 addr, u32 value);

private:
    u8 control;
};

} // namespace iop