#pragma once

#include "common/types.h"
#include "common/log.h"
#include "core/iop/cpu_regs.h"
#include "core/iop/cop0.h"
#include "core/iop/interrupt_controller.h"

class System;

class IOPCore {
public:
    IOPCore(System* system);

    virtual void Reset() = 0;
    virtual void Run(int cycles) = 0;

    u32 GetReg(int reg) {
        return regs.gpr[reg];
    }

    void SetReg(int reg, u32 data) {
        if (reg) {
            regs.gpr[reg] = data;
        }
    }

    void EnableHardwareInterrupt();
    void DisableHardwareInterrupt();

    IOPRegs regs;
    IOPCOP0 cop0;
    System* system;
    IOPInterruptController interrupt_controller;
    // FILE* fp = fopen("../../log-stuff/iop1.log", "w");
};