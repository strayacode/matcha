#include <string.h>
#include "core/elf_loader.h"
#include "core/system.h"

ELFLoader::ELFLoader(System& system) : system(system) {}

ELFLoader::~ELFLoader() {
    if (elf) {
        delete[] elf;
    }
}

void ELFLoader::Load() {
    std::ifstream file(path, std::ios::binary);

    if (!file) {
        common::Error("[ELFLoader] elf with path %s does not exist!", path.c_str());
    }

    file.unsetf(std::ios::skipws);
    file.seekg(0, std::ios::end);
    size = file.tellg();
    elf = new u8[size];
    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char*>(elf), size);
    file.close();

    common::Debug("[ELFLoader] ELF was successfully loaded!");
    common::Debug("[ELFLoader] Size: %08x", size);

    LoadHeader();
}

void ELFLoader::LoadHeader() {
    // check if the elf has at least the first 4 magic bytes
    if (elf[0] != 0x7F || elf[1] != 'E' || elf[2] != 'L' || elf[3] != 'F') {
        common::Error("[ELFLoader] invalid ELF! %s", path.c_str());
    }

    memcpy(&header, &elf[0x10], sizeof(ELFHeader));

    for (u32 header_offset = header.phoff; header_offset < header.phoff + (header.phnum * header.phentsize); header_offset += header.phentsize) {
        // now offset is at the start of a program header
        ProgramHeader program_header;
        memcpy(&program_header, &elf[header_offset], sizeof(ProgramHeader));
        
        // now copy the segment into the EEBus word by word
        for (u32 program_offset = program_header.offset; program_offset < (program_header.offset + program_header.filesz); program_offset += 4) {
            u32 data = 0;
            memcpy(&data, &elf[program_offset], 4);

            system.memory.EEWriteWord(program_header.paddr, data);
            program_header.paddr += 4;
        }
    }

    common::Debug("[ELFLoader] entrypoint: %08x", header.entry);
    
    system.ee.pc = header.entry;
}

void ELFLoader::SetPath(std::string elf_path) {
    path = elf_path;
}