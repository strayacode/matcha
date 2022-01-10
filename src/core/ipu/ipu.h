#pragma once

#include <common/types.h>
#include <common/log.h>

class IPU {
public:
    void Reset();
    void SystemReset();

    void WriteControl(u32 data);
    u32 ReadControl();
    void WriteCommand(u32 data);
private:
    u32 control;
    u32 command;
};