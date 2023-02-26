#pragma once

#include "common/log.h"
#include "core/ee/context.h"
#include <core/memory/memory.h>
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
#include <core/iop/cpu_core.h>
#include <core/iop/interpreter/interpreter.h>
#include "core/iop/dmac.h"
#include "core/iop/timers.h"
#include "core/elf_loader.h"
#include "core/spu/spu.h"
#include <memory>
#include <array>

enum class CoreType {
    Interpreter,
};

struct System {
    System();

    void Reset();
    void InitialiseIOPCore(CoreType core_type);
    void RunFrame();
    void SingleStep();
    void VBlankStart();
    void VBlankFinish();
    void SetGamePath(std::string path);
    void LoadBIOS();

    Scheduler scheduler;

    Memory memory;
    ee::Context ee;
    std::unique_ptr<IOPCore> iop_core;
    
    IOPDMAC iop_dmac;
    IOPTimers iop_timers;
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

    std::function<void()> VBlankStartEvent;
    std::function<void()> VBlankFinishEvent;
};