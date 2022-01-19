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
    virtual ~IOPCore() {}

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

    u8 ReadByte(u32 addr);
    u16 ReadHalf(u32 addr);
    u32 ReadWord(u32 addr);

    void WriteByte(u32 addr, u8 data);
    void WriteHalf(u32 addr, u16 data);
    void WriteWord(u32 addr, u32 data);

    void SendInterruptSignal(bool value);
    void CheckInterrupts();

    enum class ExceptionType {
        Interrupt = 0x00,
        LoadError = 0x04,
        StoreError = 0x05,
        Syscall = 0x08,
        Break = 0x09,
        Reserved = 0x0A,
        Overflow = 0x0C,
    };

    void DoException(ExceptionType exception);

    IOPRegs regs;
    IOPCOP0 cop0;
    System* system;
    IOPInterruptController interrupt_controller;
    // FILE* fp = fopen("../../log-stuff/iop1.log", "w");

    bool branch_delay;
    bool branch;
};