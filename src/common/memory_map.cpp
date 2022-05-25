#include <assert.h>
#include "common/memory_map.h"
#include "common/log_file.h"

void MemoryMap::RegisterReadMemory(u32 start, u32 end, u8* pointer) {
    MemoryReadElement read_element;
    read_element.start = start;
    read_element.end = end;
    read_element.pointer = pointer;
    read_element.type = MemoryType::Fast;

    read_map.push_back(read_element);
}

void MemoryMap::RegisterWriteMemory(u32 start, u32 end, u8* pointer) {
    MemoryWriteElement write_element;
    write_element.start = start;
    write_element.end = end;
    write_element.pointer = pointer;
    write_element.type = MemoryType::Fast;

    write_map.push_back(write_element);
}

void MemoryMap::RegisterReadHandler(u32 start, u32 end, MemoryReadHandler handler) {
    MemoryReadElement read_element;
    read_element.start = start;
    read_element.end = end;
    read_element.handler = handler;
    read_element.type = MemoryType::Slow;

    read_map.push_back(read_element);
}

void MemoryMap::RegisterWriteHandler(u32 start, u32 end, MemoryWriteHandler handler) {
    MemoryWriteElement write_element;
    write_element.start = start;
    write_element.end = end;
    write_element.handler = handler;
    write_element.type = MemoryType::Slow;

    write_map.push_back(write_element);
}

MemoryMap::MemoryReadElement* MemoryMap::GetReadMap(u32 addr) {
    for (int i = 0; i < read_map.size(); i++) {
        if (addr >= read_map[i].start && addr < read_map[i].end) {
            return &read_map[i];
        }
    }

    return nullptr;
}

MemoryMap::MemoryWriteElement* MemoryMap::GetWriteMap(u32 addr) {
    for (int i = 0; i < write_map.size(); i++) {
        if (addr >= write_map[i].start && addr < write_map[i].end) {
            return &write_map[i];
        }
    }

    return nullptr;
}

u8 MemoryMap::ReadByte(u32 addr) {
    MemoryReadElement* read_element = GetReadMap(addr);

    if (!read_element) {
        LogFile::Get().Log("[MemoryMap] %08x is not mapped\n", addr);
        return 0;
    }

    switch (read_element->type) {
    case MemoryType::Fast:
        return read_element->pointer[addr - read_element->start];
    case MemoryType::Slow:
        return read_element->handler(addr);
    }

    return 0;
}

u16 MemoryMap::ReadHalf(u32 addr) {
    MemoryReadElement* read_element = GetReadMap(addr);

    if (!read_element) {
        LogFile::Get().Log("[MemoryMap] %08x is not mapped\n", addr);
        return 0;
    }

    switch (read_element->type) {
    case MemoryType::Fast:
        return *reinterpret_cast<u16*>(&read_element->pointer[addr - read_element->start]);
    case MemoryType::Slow:
        return read_element->handler(addr);
    }

    return 0;
}

u32 MemoryMap::ReadWord(u32 addr) {
    MemoryReadElement* read_element = GetReadMap(addr);

    if (!read_element) {
        LogFile::Get().Log("[MemoryMap] %08x is not mapped\n", addr);
        return 0;
    }

    switch (read_element->type) {
    case MemoryType::Fast:
        return *reinterpret_cast<u32*>(&read_element->pointer[addr - read_element->start]);
    case MemoryType::Slow:
        return read_element->handler(addr);
    }

    return 0;
}

void MemoryMap::WriteByte(u32 addr, u8 data) {
    MemoryWriteElement* write_element = GetWriteMap(addr);

    if (!write_element) {
        LogFile::Get().Log("[MemoryMap] %08x is not mapped\n", addr);
        return;
    }

    switch (write_element->type) {
    case MemoryType::Fast:
        write_element->pointer[addr - write_element->start] = data;
        break;
    case MemoryType::Slow:
        write_element->handler(addr, data);
        break;
    }
}

void MemoryMap::WriteHalf(u32 addr, u16 data) {
    MemoryWriteElement* write_element = GetWriteMap(addr);

    if (!write_element) {
        LogFile::Get().Log("[MemoryMap] %08x is not mapped\n", addr);
        return;
    }

    switch (write_element->type) {
    case MemoryType::Fast:
        *reinterpret_cast<u16*>(&write_element->pointer[addr - write_element->start]) = data;
        break;
    case MemoryType::Slow:
        write_element->handler(addr, data);
        break;
    }
}

void MemoryMap::WriteWord(u32 addr, u32 data) {
    MemoryWriteElement* write_element = GetWriteMap(addr);

    if (!write_element) {
        LogFile::Get().Log("[MemoryMap] %08x is not mapped\n", addr);
        return;
    }

    switch (write_element->type) {
    case MemoryType::Fast:
        *reinterpret_cast<u32*>(&write_element->pointer[addr - write_element->start]) = data;
        break;
    case MemoryType::Slow:
        write_element->handler(addr, data);
        break;
    }
}