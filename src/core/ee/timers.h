#pragma once

#include "common/types.h"
#include "common/log.h"

struct TimerChannel {
    u32 counter;
    u16 control;
    u16 compare;
    u16 hold;
    int cycles;
    int cycles_per_tick;
};

class System;

class Timers {
public:
    Timers(System& system);

    void Reset();
    void WriteRegister(u32 addr, u32 data);
    
    u32 ReadChannel(u32 addr);

    void Increment(int index);
    void Run(int cycles);
    
private:
    TimerChannel channels[4];
    System& system;
};