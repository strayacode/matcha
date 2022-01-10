#pragma once

#include <common/types.h>
#include <common/log.h>

class System;

class GS {
public:
    GS(System* system);

    void Reset();
    void SystemReset();

    void WriteCSR(u32 data);
    void WriteSMODE1(u64 data);
    void WriteSYNCH1(u64 data);
    void WriteSYNCH2(u64 data);
    void WriteSYNCV(u64 data);
    void WriteSMODE2(u8 data);
    void WriteSRFSH(u64 data);
private:
    u32 csr;

    // this register seems to be undocumented
    u64 smode1;
    u64 synch1;
    u64 synch2;
    u64 syncv;
    u64 srfsh;

    u8 smode2;

    System* system;
};