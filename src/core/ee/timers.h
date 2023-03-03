#pragma once

#include "common/types.h"
#include "common/log.h"
#include "core/ee/intc.h"

namespace ee {

class Timers {
public:
    Timers(INTC& intc);

    void Reset();
    u32 ReadRegister(u32 addr);
    void WriteRegister(u32 addr, u32 data);
    
    u32 ReadChannel(u32 addr);

    void Increment(int index);
    void Run(int cycles);
    
private:
    struct Channel {
        u32 counter;
        u16 control;
        u16 compare;
        u16 hold;
        int cycles;
        int cycles_per_tick;
    };

    Channel channels[4];
    INTC& intc;
};

} // namespace ee