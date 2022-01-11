#pragma once

#include "common/log.h"
#include "core/ee/ee_core.h"
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
#include <memory>

enum class CoreType {
    Interpreter,
};

class System {
public:
    System();

    void Reset();
    void InitialiseIOPCore(CoreType core_type);
    void RunFrame();
    void VBlankStart();
    void VBlankFinish();
    void FastBoot();

    Scheduler scheduler;

    EECore ee_core;
    std::unique_ptr<IOPCore> iop_core;
    Memory memory;
    IOPDMAC iop_dmac;
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

    std::function<void()> VBlankStartEvent;
    std::function<void()> VBlankFinishEvent;
};