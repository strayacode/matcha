#pragma once

#include "common/types.h"
#include "common/cpu_types.h"
#include "core/ee/cpu_regs.h"
#include "core/ee/cop0.h"

class System;

class EECore {
public:
    EECore(System& system);

    void Reset();
    void Run(int cycles);

    // credit goes to DobieStation for the elegant way of accessing 128 bit registers
    template <typename T>
    T GetReg(int reg) {
        return *(T*)&regs.gpr[reg * sizeof(u64) * 2];
    }

    template <typename T>
    T GetReg(int reg, int offset) {
        return *(T*)&regs.gpr[(reg * sizeof(u64) * 2) + (offset * sizeof(T))];
    }

    template <typename T>
    void SetReg(int reg, T data) {
        if (reg) {
            *(T*)&regs.gpr[reg * sizeof(u64) * 2] = data;
        }
    }

    template <typename T>
    void SetReg(int reg, T data, int offset) {
        if (reg) {
            *(T*)&regs.gpr[(reg * sizeof(u64) * 2) + (offset * sizeof(T))] = data;
        }
    }

    EERegs regs;
    System& system;
    EECOP0 ee_cop0;
    bool branch_delay;
    bool branch;
    CPUInstruction inst;
};