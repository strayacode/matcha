#pragma once

#include <functional>
#include <vector>
#include "common/types.h"

// this gets used by the ee and iop for their memory maps
class MemoryMap {
public:
    using MemoryReadHandler = std::function<u32(u32)>;
    using MemoryWriteHandler = std::function<void(u32, u32)>;

    enum class MemoryType {
        Fast,
        Slow,
    };

    struct MemoryReadElement {
        u32 start;
        u32 end;
        u8* pointer;
        MemoryReadHandler handler;
        MemoryType type;
    };

    struct MemoryWriteElement {
        u32 start;
        u32 end;
        u8* pointer;
        MemoryWriteHandler handler;
        MemoryType type;
    };

    using MemoryReadMap = std::vector<MemoryReadElement>;
    using MemoryWriteMap = std::vector<MemoryWriteElement>;

    u8 ReadByte(u32 addr);
    u16 ReadHalf(u32 addr);
    u32 ReadWord(u32 addr);

    void WriteByte(u32 addr, u8 data);
    void WriteHalf(u32 addr, u16 data);
    void WriteWord(u32 addr, u32 data);

    // TODO: get rid of the repitition by just using a MemoryElement struct
    void RegisterReadMemory(u32 start, u32 end, u8* pointer);
    void RegisterWriteMemory(u32 start, u32 end, u8* pointer);
    void RegisterReadHandler(u32 start, u32 end, MemoryReadHandler handler);
    void RegisterWriteHandler(u32 start, u32 end, MemoryWriteHandler handler);

private:
    MemoryReadElement* GetReadMap(u32 addr);
    MemoryWriteElement* GetWriteMap(u32 addr);
    
    MemoryReadMap read_map;
    MemoryWriteMap write_map;
};