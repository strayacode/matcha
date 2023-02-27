#include "common/log.h"
#include "core/memory/memory.h"
#include "core/memory/memory_constants.h"
#include "core/system.h"

Memory::Memory(System* system) : system(system) {}

Memory::~Memory() {
    if (iop_ram) {
        delete[] iop_ram;
    }
}

void Memory::Reset() {
    iop_table.fill(nullptr);

    InitialiseMemory();
    RegisterRegion(0x1FC00000, 0x20000000, 0x3FFFFF, system->bios->data(), RegionType::IOP);
    RegisterRegion(0x0000000, 0x200000, 0x1FFFFF, iop_ram, RegionType::IOP);
}

bool Memory::InRange(u32 base, u32 size, u32 addr) {
    return (addr >= base) && (addr < (base + size));
}

// TODO: check if this affects performance
bool Memory::ValidEECodeRegion(VirtualAddress vaddr) {
    u32 addr = vaddr & 0x1FFFFFFF;

    // for now we just assume code is in main ram or the bios
    return (InRange(RDRAM_BASE, RDRAM_SIZE, addr) || InRange(BIOS_BASE, BIOS_SIZE, addr));
}

bool Memory::ValidIOPCodeRegion(VirtualAddress vaddr) {
    u32 addr = vaddr & 0x1FFFFFFF;

    // for now we just assume code is in iop ram or the bios
    return (InRange(IOP_RAM_BASE, IOP_RAM_SIZE, addr) || InRange(BIOS_BASE, BIOS_SIZE, addr));
}

void Memory::InitialiseMemory() {
    iop_ram = new u8[0x200000];
}

void Memory::RegisterRegion(VirtualAddress vaddr_start, VirtualAddress vaddr_end, int mask, u8* region, RegionType region_type) {
    for (u32 vaddr = vaddr_start; vaddr < vaddr_end; vaddr += 0x1000) {
        int index = PageIndex(vaddr);

        if (region_type == RegionType::EE) {
            ee_table[index] = &region[vaddr & mask];
        } else {
            iop_table[index] = &region[vaddr & mask];
        }
    }
}

int Memory::PageIndex(VirtualAddress vaddr) {
    return vaddr >> 12;
}

int Memory::PageOffset(VirtualAddress vaddr) {
    return vaddr & 0xFFF;
}

template u8 Memory::IOPRead(VirtualAddress vaddr);
template u16 Memory::IOPRead(VirtualAddress vaddr);
template u32 Memory::IOPRead(VirtualAddress vaddr);
template <typename T>
T Memory::IOPRead(VirtualAddress vaddr) {
    T return_value = 0;
    u32 addr = vaddr & 0x1FFFFFFF;
    u8* page = iop_table[PageIndex(addr)];

    if (page) {
        memcpy(&return_value, page + PageOffset(addr), sizeof(T));
    } else {
        if constexpr (sizeof(T) == 1) {
            return IOPReadByte(addr);
        } else if constexpr (sizeof(T) == 2) {
            return IOPReadHalf(addr);
        } else {
            return IOPReadWord(addr);
        }
    }

    return return_value;
}

u8 Memory::IOPReadByte(u32 addr) {
    if (addr >= 0x1f402004 && addr < 0x1f402019) {
        return system->iop_core->cdvd.ReadRegister(addr);
    }

    switch (addr) {
    default:
        common::Log("[iop::Context] handle iop byte read %08x", addr);
    }

    return 0;
}

u16 Memory::IOPReadHalf(u32 addr) {
    if ((addr >= IOP_TIMERS_REGION1_START && addr < IOP_TIMERS_REGION1_END) ||
        (addr >= IOP_TIMERS_REGION2_START && addr < IOP_TIMERS_REGION2_END)) {
        return system->iop_timers.ReadRegister(addr);
    } else if ((addr >= SPU_REGION1_START && addr < SPU_REGION1_END) ||
        (addr >= SPU_REGION2_START && addr < SPU_REGION2_END)) {
        return 0;
    } else if (addr >= SPU2_REGION_START && addr < SPU2_REGION_END) {
        return system->spu2.ReadRegister(addr);
    } else {
        common::Log("[iop::Context] handle iop half read %08x", addr);
    }

    return 0;
}

u32 Memory::IOPReadWord(u32 addr) {
    if ((addr >= IOP_DMA_REGION1_START && addr < IOP_DMA_REGION1_END) ||
        (addr >= IOP_DMA_REGION2_START && addr < IOP_DMA_REGION2_END) ||
        (addr >= IOP_DMA_REGION3_START && addr < IOP_DMA_REGION3_END)) {
        return system->iop_dmac.ReadRegister(addr);
    } else if ((addr >= IOP_TIMERS_REGION1_START && addr < IOP_TIMERS_REGION1_END) ||
        (addr >= IOP_TIMERS_REGION2_START && addr < IOP_TIMERS_REGION2_END)) {
        return system->iop_timers.ReadRegister(addr);
    } else if ((addr >> 24) == 0x1E) {
        // what is this
        return 0;
    }

    switch (addr) {
    case 0x1D000010:
        return system->sif.ReadSMCOM();
    case 0x1D000020:
        return system->sif.ReadMSFLAG();
    case 0x1D000030:
        return system->sif.ReadSMFLAG();
    case 0x1D000040:
        return system->sif.ReadControl();
    case 0x1D000060:
        return system->sif.bd6;
    case 0x1F80100C:
    case 0x1F801010:
    case 0x1F801400:
    case 0x1F801450:
    case 0x1F801070:
        return system->iop_core->interrupt_controller.ReadRegister(0);
    case 0x1F801074:
        return system->iop_core->interrupt_controller.ReadRegister(4);
    case 0x1F801078:
        return system->iop_core->interrupt_controller.ReadRegister(8);
    default:
        common::Log("[iop::Context] handle iop word read %08x", addr);
    }

    return 0;
}

template void Memory::IOPWrite(VirtualAddress vaddr, u8 data);
template void Memory::IOPWrite(VirtualAddress vaddr, u16 data);
template void Memory::IOPWrite(VirtualAddress vaddr, u32 data);
template <typename T>
void Memory::IOPWrite(VirtualAddress vaddr, T data) {
    u32 addr = vaddr & 0x1FFFFFFF;
    u8* page = iop_table[PageIndex(addr)];

    if (page) {
        memcpy(page + PageOffset(addr), &data, sizeof(T));
    } else {
        if constexpr (sizeof(T) == 1) {
            IOPWriteByte(addr, data);
        } else if constexpr (sizeof(T) == 2) {
            IOPWriteHalf(addr, data);
        } else {
            IOPWriteWord(addr, data);
        }
    }
}

void Memory::IOPWriteByte(u32 addr, u8 data) {
    switch (addr) {
    case 0x1F802070:
        return;
    default:
        common::Log("[iop::Context] handle iop byte write %08x = %02x", addr, data);
        break;
    }
}

void Memory::IOPWriteHalf(u32 addr, u16 data) {
    if ((addr >= IOP_DMA_REGION1_START && addr < IOP_DMA_REGION1_END) ||
        (addr >= IOP_DMA_REGION2_START && addr < IOP_DMA_REGION2_END) ||
        (addr >= IOP_DMA_REGION3_START && addr < IOP_DMA_REGION3_END)) {
        system->iop_dmac.WriteRegister(addr, data);
        return;
    } else if ((addr >= IOP_TIMERS_REGION1_START && addr < IOP_TIMERS_REGION1_END) ||
        (addr >= IOP_TIMERS_REGION2_START && addr < IOP_TIMERS_REGION2_END)) {
        system->iop_timers.WriteRegister(addr, data);
        return;
    } else if ((addr >= SPU_REGION1_START && addr < SPU_REGION1_END) ||
        (addr >= SPU_REGION2_START && addr < SPU_REGION2_END)) {
        system->spu.WriteRegister(addr, data);
        return;
    } else if (addr >= SPU2_REGION_START && addr < SPU2_REGION_END) {
        system->spu2.WriteRegister(addr, data);
        return;
    } else {
        common::Log("[iop::Context] handle iop half write %08x = %04x", addr, data);
    }
}

void Memory::IOPWriteWord(u32 addr, u32 data) {
    if ((addr >= IOP_DMA_REGION1_START && addr < IOP_DMA_REGION1_END) ||
        (addr >= IOP_DMA_REGION2_START && addr < IOP_DMA_REGION2_END) ||
        (addr >= IOP_DMA_REGION3_START && addr < IOP_DMA_REGION3_END)) {
        system->iop_dmac.WriteRegister(addr, data);
        return;
    } else if ((addr >= IOP_TIMERS_REGION1_START && addr < IOP_TIMERS_REGION1_END) ||
        (addr >= IOP_TIMERS_REGION2_START && addr < IOP_TIMERS_REGION2_END)) {
        system->iop_timers.WriteRegister(addr, data);
        return;
    }

    switch (addr) {
    case 0x1D000010:
        system->sif.WriteSMCOM(data);
        break;
    case 0x1D000020:
        system->sif.ResetMSFLAG(data);
        break;
    case 0x1D000030:
        system->sif.SetSMFLAG(data);
        break;
    case 0x1D000040:
        system->sif.WriteIOPControl(data);
        break;
    case 0x1F801004:
    case 0x1F80100C:
    case 0x1F801010:
    case 0x1F801014:
    case 0x1F801018:
    case 0x1F80101C:
    case 0x1F801020:
    case 0x1F801400:
    case 0x1F801404:
    case 0x1F801408:
    case 0x1F80140C:
    case 0x1F801410:
    case 0x1F801414:
    case 0x1F801418:
    case 0x1F80141C:
    case 0x1F801420:
    case 0x1F802070:
    case 0x1F801060:
    case 0x1F801450:
    case 0x1F801560:
    case 0x1F801564:
    case 0x1F801568:
        // undocumented
        break;
    // TODO: clean this
    case 0x1F801070:
        system->iop_core->interrupt_controller.WriteRegister(0, data);
        break;
    case 0x1F801074:
        system->iop_core->interrupt_controller.WriteRegister(4, data);
        break;
    case 0x1F801078:
        system->iop_core->interrupt_controller.WriteRegister(8, data);
        break;
    case 0x1F8015F0:
        break;
    default:
        common::Log("[iop::Context] handle iop word write %08x = %08x", addr, data);
    }
}