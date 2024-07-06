#pragma once

#include "common/types.h"

#include "core/iop/intc.h"

namespace iop {

class Timers {
public:
    Timers(INTC& intc);

    void reset();
    void run(int cycles);
    u32 read(u32 addr);
    void write(u32 addr, u32 data);

private:
    int calculate_channel_index(u32 addr);
    u32 calculate_channel_prescaler(int index);
    void run_channel(int index, int cycles);
    bool overflow_occured(int index);
    void raise_irq(int index);

    struct Channel {
        u64 counter;
        u64 cycles;
        u64 cycles_per_tick;

        union Mode {
            struct {
                bool gate_enable : 1;
                u32 gate_mode : 2;

                // Reset counter on compare interrupts.
                u32 zero_return : 1;

                bool compare_irq : 1;
                bool overflow_irq : 1;
                bool repeat_irq : 1;
                bool levl : 1;

                // If set:
                // Timer 0: pixel clock (13.5mhz)
                // Timer 1/3: hblank
                // Ohter: sysclock (just regular clock)
                bool use_external_signal : 1;

                bool timer2_prescaler : 1;
                bool irqs_enabled : 1;
                bool compare_irq_raised : 1;
                bool overflow_irq_raised : 1;
                u32 timer45_prescaler : 2;
                u32 : 17;
            };

            u32 data;
        } mode;

        u32 target;
    } m_channels[6];

    INTC& m_intc;

    static constexpr u32 EE_CLOCK = 294912000;
    static constexpr u32 IOP_CLOCK = EE_CLOCK / 8;
};

} // namespace iop