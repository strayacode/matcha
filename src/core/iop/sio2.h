#pragma once

#include <array>
#include "common/types.h"
#include "common/queue.h"
#include "core/iop/intc.h"

namespace iop {

struct SIO2 {
    SIO2(INTC& intc);

    void Reset();

    u32 ReadRegister(u32 addr);
    void WriteRegister(u32 addr, u32 value);
    u8 ReadDMA();
    void WriteDMA(u8 data);

private:
    u8 control;
    std::array<u32, 4> send1;
    std::array<u32, 4> send2;
    std::array<u32, 16> send3;
    common::Queue<u8, 256> fifo;
    INTC& intc;
};

} // namespace iop