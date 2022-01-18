#pragma once

#include "common/types.h"
#include "common/cpu_types.h"
#include "common/int128.h"
#include "core/ee/cop0.h"
#include "core/ee/cop1.h"
#include "core/ee/interpreter_table.h"

class System;

enum class ExceptionType : int {
    Interrupt = 0,
    TLBModified = 1,
    TLBRefillInstruction = 2,
    TLBRefillStore = 3,
    AddressErrorInstruction = 4,
    AddressErrorStore = 5,
    BusErrorInstruction = 6,
    BusErrorStore = 7,
    Syscall = 8,
    Break = 9,
    Reserved = 10,
    CoprocessorUnusable = 11,
    Overflow = 12,
    Trap = 13,
    Reset = 14,
    NMI = 15,
    PerformanceCounter = 16,
    Debug = 18,
};

class EECore {
public:
    EECore(System& system);

    void Reset();
    void Run(int cycles);

    // credit goes to DobieStation for the elegant way of accessing 128 bit registers
    template <typename T>
    T GetReg(int reg) {
        return *(T*)&gpr[reg * sizeof(u64) * 2];
    }

    template <typename T>
    T GetReg(int reg, int offset) {
        return *(T*)&gpr[(reg * sizeof(u64) * 2) + (offset * sizeof(T))];
    }

    template <typename T>
    void SetReg(int reg, T data) {
        if (reg) {
            *(T*)&gpr[reg * sizeof(u64) * 2] = data;
        }
    }

    template <typename T>
    void SetReg(int reg, T data, int offset) {
        if (reg) {
            *(T*)&gpr[(reg * sizeof(u64) * 2) + (offset * sizeof(T))] = data;
        }
    }

    u8 ReadByte(u32 addr);
    u16 ReadHalf(u32 addr);
    u32 ReadWord(u32 addr);
    u64 ReadDouble(u32 addr);

    void WriteByte(u32 addr, u8 data);
    void WriteHalf(u32 addr, u16 data);
    void WriteWord(u32 addr, u32 data);
    void WriteDouble(u32 addr, u64 data);
    void WriteQuad(u32 addr, u128 data);

    void DoException(u32 target, ExceptionType exception);
    void SendInterruptSignal(int signal, bool value);
    void CheckInterrupts();
    bool InterruptsEnabled();
    void PrintRegs();

    u8 gpr[32 * sizeof(u64) * 2];
    u32 pc;
    u32 next_pc;
    u64 hi;
    u64 lo;
    u64 hi1;
    u64 lo1;
    u64 sa;

    System& system;
    EECOP0 cop0;
    EECOP1 cop1;
    bool branch_delay;
    bool branch;
    CPUInstruction inst;
    InterpreterTable interpreter_table;

    FILE* fp = fopen("../../log-stuff/ps2.log", "w");
};