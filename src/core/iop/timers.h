#pragma once

#include "common/types.h"

class System;

class IOPTimers {
public:
    IOPTimers(System& system);
    void Reset();
    void Run(int cycles);
    u32 ReadRegister(u32 addr);
    void WriteRegister(u32 addr, u32 data);
    int GetTimerIndex(u32 addr);

    struct Channel {
        u32 counter;
        u32 mode;
        u32 target;
    } channels[6];
    
private:
    System& system;
};