#pragma once

#include <string.h>
#include <array>
#include <memory>
#include <fstream>
#include "common/types.h"
#include "common/log.h"
#include "common/memory_helpers.h"
#include "common/memory_map.h"

struct System;

enum class RegionType {
    EE,
    IOP,
};

class Memory {
public:
    Memory(System* system);
    ~Memory();

    void Reset();
    bool InRange(u32 base, u32 size, u32 addr);
    bool ValidEECodeRegion(VirtualAddress vaddr);
    bool ValidIOPCodeRegion(VirtualAddress vaddr);
    void InitialiseMemory();
    u32 TranslateVirtualAddress(VirtualAddress vaddr);
    void RegisterRegion(VirtualAddress vaddr_start, VirtualAddress vaddr_end, int mask, u8* region, RegionType region_type);
    int PageIndex(VirtualAddress vaddr);
    int PageOffset(VirtualAddress vaddr);

    u8 EEReadByte(u32 addr);
    u16 EEReadHalf(u32 addr);
    u32 EEReadWord(u32 addr);
    u64 EEReadDouble(u32 addr);

    void EEWriteByte(u32 addr, u8 data);
    void EEWriteHalf(u32 addr, u16 data);
    void EEWriteWord(u32 addr, u32 data);
    void EEWriteDouble(u32 addr, u64 data);
    void EEWriteQuad(u32 addr, u128 data);

    u32 EEReadIO(u32 addr);
    void EEWriteIO(u32 addr, u32 data);

    template <typename T>
    T IOPRead(VirtualAddress vaddr);

    u8 IOPReadByte(u32 addr);
    u16 IOPReadHalf(u32 addr);
    u32 IOPReadWord(u32 addr);

    template <typename T>
    void IOPWrite(VirtualAddress vaddr, T data);

    void IOPWriteByte(u32 addr, u8 data);
    void IOPWriteHalf(u32 addr, u16 data);
    void IOPWriteWord(u32 addr, u32 data);

    // 0x00000000 - 0x02000000 32MB RDRAM
    // (first 1MB reserved for the kernel)
    u8* rdram;

    // 0x1C000000 - 0x1C200000 2MB IOP RAM
    u8* iop_ram;

    // 0x1FC00000 - 0x20000000 4MB PS2 BIOS
    // is used for both the ee and iop
    u8* bios;

    // 0x70000000 - 0x70004000 16KB scratchpad ram
    // only accessible via virtual addressing and is faster
    u8* scratchpad;

    std::array<u8*, 0x100000> ee_table;
    std::array<u8*, 0x100000> iop_table;

    // no clue what this register does
    u32 mch_drd;

    // same with this one
    u32 rdram_sdevid;

    // same with this one
    u32 mch_ricm;

    System* system;

    MemoryMap ee_map;
};