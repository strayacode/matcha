#include <core/system.h>

System::System() : ee_core(*this), memory(this), iop_dmac(*this), iop_timers(*this), ee_intc(*this), gif(this), gs(this), timers(*this), dmac(this), elf_loader(*this) {
    VBlankStartEvent = std::bind(&System::VBlankStart, this);
    VBlankFinishEvent = std::bind(&System::VBlankFinish, this);
    InitialiseIOPCore(CoreType::Interpreter);
}

// credit goes to pcsx2
// NTSC Interlaced Timings
#define CYCLES_PER_FRAME 4920115 // 4920115.2 EE cycles to be exact FPS of 59.94005994005994hz
#define VBLANK_START_CYCLES 4489019 // 4489019.391883126 Guess, exactly 23 HBLANK's before the end
#define HBLANK_CYCLES 18742
#define GS_VBLANK_DELAY 65622 // CSR FIELD swap/vblank happens ~65622 cycles after the INTC VBLANK_START event

// clockrates
#define EE_CLOCK_SPEED 294912000
#define BUS_CLOCK_SPEED EE_CLOCK_SPEED / 2
#define IOP_CLOCK_SPEED EE_CLOCK_SPEED / 8

void System::Reset() {
    scheduler.Reset();
    ee_core.Reset();
    iop_core->Reset();
    memory.Reset();
    iop_dmac.Reset();
    iop_timers.Reset();
    ee_intc.Reset();
    gif.Reset();
    gs.Reset();
    timers.Reset();
    dmac.Reset();
    vu0.Reset();
    vu1.Reset();
    vif0.Reset();
    vif1.Reset();
    ipu.Reset();
    sif.Reset();
}

void System::InitialiseIOPCore(CoreType core_type) {
    if (core_type == CoreType::Interpreter) {
        iop_core = std::make_unique<IOPInterpreter>(this);
    } else {
        log_fatal("[System] Unknown core type");
    }
}

void System::RunFrame() {
    u64 end_timestamp = scheduler.GetCurrentTime() + CYCLES_PER_FRAME;
    int cycles = 32;
    scheduler.Add(VBLANK_START_CYCLES, VBlankStartEvent);
    scheduler.Add(CYCLES_PER_FRAME, VBlankFinishEvent);

    while (scheduler.GetCurrentTime() < end_timestamp) {
        ee_core.Run(cycles);
        
        // ee timers and dmac run at half the speed of the ee
        timers.Run(cycles / 2);
        dmac.Run(cycles / 2);

        // iop runs at 1 / 8 speed of the ee
        iop_core->Run(cycles / 8);
        iop_dmac.Run(cycles / 8);
        iop_timers.Run(cycles / 8);
        
        scheduler.Tick(cycles);
        scheduler.RunEvents();
    }
}

// implement later
void System::SingleStep() {}

void System::VBlankStart() {
    ee_intc.RequestInterrupt(EEInterruptSource::VBlankStart);
    iop_core->interrupt_controller.RequestInterrupt(IOPInterruptSource::VBlankStart);
}

void System::VBlankFinish() {
    ee_intc.RequestInterrupt(EEInterruptSource::VBlankFinish);
    iop_core->interrupt_controller.RequestInterrupt(IOPInterruptSource::VBlankFinish);
}

void System::SetGamePath(std::string path) {
    elf_loader.SetPath(path);
}