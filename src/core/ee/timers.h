#pragma once

#include <common/types.h>
#include <common/log.h>

struct TimerChannel {
    u32 counter;
    u16 control;
    u16 compare;
    u16 hold;
    int ticks;
};

class Timers {
public:
    void Reset();

    void WriteChannel(u32 addr, u32 data);
    u32 ReadChannel(u32 addr);

    void WriteControl(int index, u16 data);
    void WriteCounter(int index, u16 data);
    void WriteCompare(int index, u16 data);
    void WriteHold(int index, u16 data);

    void Tick(int index, int ticks);

    void Run(int cycles);
private:
    TimerChannel channels[4];
};