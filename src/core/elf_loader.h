#pragma once

#include <string>
#include <fstream>
#include "common/types.h"
#include "common/log.h"

class System;

class ELFLoader {
public:
    ELFLoader(System& system);
    ~ELFLoader();
    void Load();
    void LoadHeader();
    void SetPath(std::string elf_path);

private:
    struct ELFHeader {
        u16 type;
        u16 machine;
        u32 version;
        u32 entry;
        u32 phoff;
        u32 shoff;
        u32 flags;
        u16 ehsize;
        u16 phentsize;
        u16 phnum;
        u16 shentsize;
        u16 shnum;
        u16 shstrndx;
    };

    struct ProgramHeader {
        u32 type;
        u32 offset;
        u32 vaddr;
        u32 paddr;
        u32 filesz;
        u32 memsz;
        u32 flags;
        u32 align;
    };

    std::string path;
    ELFHeader header;
    
    u8* elf = nullptr;
    System& system;
    int size = 0;
};