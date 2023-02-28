#pragma once

namespace iop {

enum class Exception : int {
    Interrupt = 0x00,
    LoadError = 0x04,
    StoreError = 0x05,
    Syscall = 0x08,
    Break = 0x09,
    Reserved = 0x0A,
    Overflow = 0x0C,
};

struct Executor {
    virtual ~Executor() = default;
    virtual void Reset() = 0;
    virtual void Run(int cycles) = 0;
};

} // namespace iop