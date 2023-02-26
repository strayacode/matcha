#pragma once

namespace ee {

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

struct Executor {
    virtual ~Executor() = default;
    virtual void Reset() = 0;
    virtual void Run(int cycles) = 0;
};

} // namespace ee