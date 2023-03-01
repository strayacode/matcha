#pragma once

#include <array>
#include <memory>
#include "common/log.h"
#include "core/ee/context.h"
#include <core/ee/intc.h>
#include <core/scheduler.h>
#include <core/gif/gif.h>
#include <core/gs/gs.h>
#include <core/ee/timers.h>
#include <core/ee/dmac.h>
#include <core/vu/vu.h>
#include <core/vif/vif.h>
#include <core/ipu/ipu.h>
#include <core/sif/sif.h>
#include "core/iop/context.h"
#include "core/elf_loader.h"
#include "core/spu/spu.h"

enum class BootMode {
    Fast,
    BIOS,
};

struct System {
    System();

    void Reset();
    void RunFrame();
    void SingleStep();
    void VBlankStart();
    void VBlankFinish();
    void SetBootParameters(BootMode boot_mode, std::string path);
    void LoadBIOS();

    Scheduler scheduler;

    ee::Context ee;
    iop::Context iop;
    
    EEINTC ee_intc;
    GIF gif;
    GS gs;
    Timers timers;
    DMAC dmac;
    VU vu0;
    VU vu1;
    VIF vif0;
    VIF vif1;
    IPU ipu;
    SIF sif;
    ELFLoader elf_loader;

    // 2 spu cores
    SPU spu;
    SPU spu2;

    // shared between ee and iop
    std::unique_ptr<std::array<u8, 0x400000>> bios;
    std::unique_ptr<std::array<u8, 0x200000>> iop_ram;

    std::function<void()> VBlankStartEvent;
    std::function<void()> VBlankFinishEvent;

    BootMode boot_mode;
    bool fastboot_done;
};