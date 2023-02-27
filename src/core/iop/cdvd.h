#pragma once

#include "common/types.h"

namespace iop {

struct CDVD {
    void Reset();

    u32 ReadRegister(u32 addr);
    void WriteRegister(u32 addr, u32 value);
    
private:
    u8 n_command_status;
    
    u8 s_command_status;
    u8 s_command;
};

} // namespace iop