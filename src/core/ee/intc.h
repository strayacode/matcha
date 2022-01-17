#pragma once

#include "common/types.h"
#include "common/log.h"

enum class EEInterruptSource : int {
    GS = 0,
    SBUS = 1,
    VBlankStart = 2,
    VBlankFinish = 3,
    VIF0 = 4,
    VIF1 = 5,
    VU0 = 6,
    VU1 = 7,
    IPU = 8,
    Timer0 = 9,
    Timer1 = 10,
    Timer2 = 11,
    Timer3 = 12,
    SFIFO = 13,
    VUOWatchdog = 14,
};

class System;

// the intc deals with interrupt requests
// and can send interrupts to the ee core via the int0 signal
class EEINTC {
public:
    EEINTC(System& system);

    void Reset();

    u16 ReadMask();
    u16 ReadStat();

    void WriteMask(u16 data);
    void WriteStat(u16 data);

    void CheckInterruptSignal();

    void RequestInterrupt(EEInterruptSource interrupt);
private:
    u16 mask;
    u16 stat;

    System& system;
};