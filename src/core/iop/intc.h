#pragma once

#include "common/types.h"

namespace iop {

struct Context;

enum class InterruptSource : u32 {
    VBlankStart = 0,
    GPU = 1,
    CDVD = 2,
    DMA = 3,
    Timer0 = 4,
    Timer1 = 5,
    Timer2 = 6,
    SIO0 = 7,
    SIO1 = 8,
    SPU2 = 9,
    PIO = 10,
    VBlankFinish = 11,
    DVD = 12,
    PCMCIA = 13,
    Timer3 = 14,
    Timer4 = 15,
    Timer5 = 16,
    SIO2 = 17,
    USB = 22,
};

class INTC {
public:
    INTC(Context& ctx);

    void Reset();

    u32 ReadRegister(int offset);
    void WriteRegister(int offset, u32 data);
    void RequestInterrupt(InterruptSource source);
    void UpdateInterrupts();

private:
    u32 interrupt_mask;
    u32 interrupt_status;
    u8 interrupt_control;

    static constexpr int WRITE_MASK = (1 << 26) - 1;

    Context& ctx;
};

} // namespace iop