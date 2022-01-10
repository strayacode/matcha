#pragma once

#include <common/types.h>
#include <common/log.h>

class VIF {
public:
    void Reset();
    void SystemReset();

    void WriteStat(u32 data);
    void WriteFBRST(u8 data);
    void WriteMark(u16 data);
    void WriteERR(u8 data);
private:
    u8 fbrst;
    u32 stat;
    u16 mark;
    u8 err;
};