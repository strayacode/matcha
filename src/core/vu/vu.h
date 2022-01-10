#pragma once

#include <common/types.h>
#include <common/log.h>
#include <common/int128.h>
#include <array>

class VU {
public:
    void Reset();

    template <typename T>
    void WriteDataMemory(u32 addr, T data) {
        *(T*)&data_memory[addr & 0x3FFF] = data;
    }

    template <typename T>
    void WriteCodeMemory(u32 addr, T data) {
        *(T*)&code_memory[addr & 0x3FFF] = data;
    }

private:
    std::array<u8, 0x4000> data_memory;
    std::array<u8, 0x4000> code_memory;
};