#include <core/system.h>

System::System() : ee(*this), iop(*this), gs(*this), gif(gs), elf_loader(*this) {
    bios = std::make_unique<std::array<u8, 0x400000>>();
    iop_ram = std::make_unique<std::array<u8, 0x200000>>();
    VBlankStartEvent = std::bind(&System::VBlankStart, this);
    VBlankFinishEvent = std::bind(&System::VBlankFinish, this);
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
    ee.Reset();
    iop.Reset();
    gs.Reset();
    gif.Reset();
    vu0.Reset();
    vu1.Reset();
    vif0.Reset();
    vif1.Reset();
    ipu.Reset();
    sif.Reset();
    spu.Reset();
    spu2.Reset();

    iop_ram->fill(0);
    bios->fill(0);
    LoadBIOS();

    fastboot_done = false;
}

void System::RunFrame() {
    u64 end_timestamp = scheduler.GetCurrentTime() + CYCLES_PER_FRAME;
    int cycles = 32;
    scheduler.Add(VBLANK_START_CYCLES, VBlankStartEvent);
    scheduler.Add(CYCLES_PER_FRAME, VBlankFinishEvent);

    while (scheduler.GetCurrentTime() < end_timestamp) {
        ee.Run(cycles);

        // these components run at bus speed (1 / 2 speed of ee)
        gif.Run(cycles / 2);

        // iop runs at 1 / 8 speed of the ee
        iop.Run(cycles / 8);
        
        scheduler.Tick(cycles);
        scheduler.RunEvents();
    }
}

// implement later
void System::SingleStep() {
    ee.Run(1);
}

void System::VBlankStart() {
    ee.intc.RequestInterrupt(ee::InterruptSource::VBlankStart);
    iop.intc.RequestInterrupt(iop::InterruptSource::VBlankStart);
}

void System::VBlankFinish() {
    ee.intc.RequestInterrupt(ee::InterruptSource::VBlankFinish);
    iop.intc.RequestInterrupt(iop::InterruptSource::VBlankFinish);
}

void System::SetBootParameters(BootMode boot_mode, std::string path = "") {
    if (boot_mode == BootMode::Fast) {
        elf_loader.SetPath(path);
    }
    
    this->boot_mode = boot_mode;
}

void System::LoadBIOS() {
    std::ifstream file("../bios/bios.bin", std::fstream::in | std::fstream::binary);

    if (!file) {
        common::Error("[System] bios does not exist!");
    }

    file.unsetf(std::ios::skipws);
    file.read(reinterpret_cast<char*>(bios.get()), 0x400000);
    file.close();

    common::Info("[System] bios was successfully loaded!");
}