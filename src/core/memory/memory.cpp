#include "core/memory/memory.h"
#include "core/memory/memory_constants.h"
#include "core/system.h"

Memory::Memory(System* system) : system(system) {}

Memory::~Memory() {
    if (rdram) {
        delete[] rdram;
    }

    if (iop_ram) {
        delete[] iop_ram;
    }

    if (bios) {
        delete[] bios;
    }

    if (scratchpad) {
        delete[] scratchpad;
    }
}

void Memory::Reset() {
    ee_table.fill(nullptr);
    iop_table.fill(nullptr);

    InitialiseMemory();
    LoadBIOS();
    RegisterRegion(0x00000000, 0x02000000, 0x1FFFFFF, rdram, RegionType::EE);
    RegisterRegion(0x1C000000, 0x1C200000, 0x1FFFFF, iop_ram, RegionType::EE);
    RegisterRegion(0x1FC00000, 0x20000000, 0x3FFFFF, bios, RegionType::EE);
    RegisterRegion(0x70000000, 0x70004000, 0x3FFF, scratchpad, RegionType::EE);
    RegisterRegion(0x1FC00000, 0x20000000, 0x3FFFFF, bios, RegionType::IOP);
    RegisterRegion(0x0000000, 0x200000, 0x1FFFFF, iop_ram, RegionType::IOP);

    mch_drd = 0;
    rdram_sdevid = 0;
    mch_ricm = 0;
}

bool Memory::InRange(u32 base, u32 size, u32 addr) {
    return (addr >= base) && (addr < (base + size));
}

// TODO: check if this affects performance
bool Memory::ValidEECodeRegion(VAddr vaddr) {
    u32 addr = TranslateVirtualAddress(vaddr);

    // for now we just assume code is in main ram or the bios
    return (InRange(RDRAM_BASE, RDRAM_SIZE, addr) || InRange(BIOS_BASE, BIOS_SIZE, addr));
}

bool Memory::ValidIOPCodeRegion(VAddr vaddr) {
    u32 addr = vaddr & 0x1FFFFFFF;

    // for now we just assume code is in iop ram or the bios
    return (InRange(IOP_RAM_BASE, IOP_RAM_SIZE, addr) || InRange(BIOS_BASE, BIOS_SIZE, addr));
}

void Memory::InitialiseMemory() {
    rdram = new u8[0x2000000];
    iop_ram = new u8[0x200000];
    bios = new u8[0x400000];
    scratchpad = new u8[0x4000];
}

void Memory::LoadBIOS() {
    std::ifstream file("../bios/bios.bin", std::fstream::in | std::fstream::binary);

    if (!file) {
        log_fatal("bios does not exist!");
    }

    file.unsetf(std::ios::skipws);
    file.read(reinterpret_cast<char*>(bios), 0x400000);
    file.close();

    log_debug("[Memory] Bios was successfully loaded!");
}

u32 Memory::TranslateVirtualAddress(VAddr vaddr) {
    if (in_range(0x70000000, 0x70004000, vaddr)) {
        // scratchpad is only accessible by virtual addressing
        return vaddr;
    } else if (in_range(0x30100000, 0x32000000, vaddr)) {
        return vaddr & 0x1FFFFFF;
    } else {
        return vaddr & 0x1FFFFFFF;
    }
}

void Memory::RegisterRegion(VAddr vaddr_start, VAddr vaddr_end, int mask, u8* region, RegionType region_type) {
    for (u32 vaddr = vaddr_start; vaddr < vaddr_end; vaddr += 0x1000) {
        int index = PageIndex(vaddr);

        if (region_type == RegionType::EE) {
            ee_table[index] = &region[vaddr & mask];
        } else {
            iop_table[index] = &region[vaddr & mask];
        }
    }
}

int Memory::PageIndex(VAddr vaddr) {
    return vaddr >> 12;
}

int Memory::PageOffset(VAddr vaddr) {
    return vaddr & 0xFFF;
}

template u8 Memory::EERead(VAddr vaddr);
template u16 Memory::EERead(VAddr vaddr);
template u32 Memory::EERead(VAddr vaddr);
template u64 Memory::EERead(VAddr vaddr);
template <typename T>
T Memory::EERead(VAddr vaddr) {
    T return_value = 0;

    u32 addr = TranslateVirtualAddress(vaddr);

    u8* page = ee_table[PageIndex(addr)];

    if (page) {
        memcpy(&return_value, page + PageOffset(vaddr), sizeof(T));
    } else {
        if constexpr (sizeof(T) == 1) {
            return EEReadByte(addr);
        } else if constexpr (sizeof(T) == 2) {
            return EEReadHalf(addr);
        } else {
            return EEReadWord(addr);
        }
    }

    return return_value;
}

u8 Memory::EEReadByte(u32 addr) {
    log_fatal("handle ee slow read %08x", addr);
    return 0;
}

u16 Memory::EEReadHalf(u32 addr) {
    switch (addr) {
    case 0x1F803800:
        return 0;
    default:
        log_fatal("handle ee slow half read %08x", addr);
    }
}

u32 Memory::EEReadWord(u32 addr) {
    if (in_range(0x10008000, 0x1000E000, addr)) {
        return system->dmac.ReadChannel(addr);
    }

    switch (addr) {
    case 0x10002010:
        return system->ipu.ReadControl();
    case 0x10003020:
        return system->gif.ReadStat();
    case 0x1000E000:
        return system->dmac.ReadControl();
    case 0x1000E010:
        return system->dmac.ReadInterruptStatus();
    case 0x1000E020:
        return system->dmac.ReadPriorityControl();
    case 0x1000E030:
        return system->dmac.ReadPriorityControl();
    case 0x1000F000:
        return system->ee_intc.ReadStat();
    case 0x1000F010:
        return system->ee_intc.ReadMask();
    case 0x1000F130:
        return 0;
    case 0x1000F200:
        return system->sif.ReadMSCOM();
    case 0x1000F210:
        return system->sif.ReadSMCOM();
    case 0x1000F220:
        return system->sif.ReadMSFLAG();
    case 0x1000F230:
        return system->sif.ReadSMFLAG();
    case 0x1000F400:
    case 0x1000F410:
        return 0;
    case 0x1000F430:
        // mch_ricm
        return 0;
    case 0x1000F440:
        if (!((mch_ricm >> 6) & 0xF)) {
            switch ((mch_ricm >> 16) & 0xFFF) {
            case 0x21:
                if (rdram_sdevid < 2) {
                    rdram_sdevid++;
                    return 0x1F;
                }
                return 0;
            case 0x23:
                return 0x0D0D;
            case 0x24:
                return 0x0090;
            case 0x40:
                return mch_ricm & 0x1F;
            }
        }

        break;
    case 0x1000F520:
        return system->dmac.disabled_status;
    case 0x1F80141C:
        // what is this
        return 0;
    default:
        log_fatal("handle ee slow word read %08x", addr);
    }
    
    return 0;
}

template void Memory::EEWrite(VAddr vaddr, u8 data);
template void Memory::EEWrite(VAddr vaddr, u16 data);
template void Memory::EEWrite(VAddr vaddr, u32 data);
template void Memory::EEWrite(VAddr vaddr, u64 data);
template void Memory::EEWrite(VAddr vaddr, u128 data);
template <typename T>
void Memory::EEWrite(VAddr vaddr, T data) {
    u32 addr = TranslateVirtualAddress(vaddr);

    u8* page = ee_table[PageIndex(addr)];

    if (page) {
        memcpy(page + PageOffset(vaddr), &data, sizeof(T));
    } else {
        if constexpr (sizeof(T) == 1) {
            EEWriteByte(addr, data);
        } else if constexpr (sizeof(T) == 2) {
            EEWriteHalf(addr, data);
        } else if constexpr (sizeof(T) == 4) {
            EEWriteWord(addr, data);
        } else if constexpr (sizeof(T) == 8) {
            EEWriteDouble(addr, data);
        } else {
            EEWriteQuad(addr, data);
        }
    }
}

void Memory::EEWriteByte(u32 addr, u8 data) {
    switch (addr) {
    case 0x1000F180:
        // kputchar
        printf("%c", data);
        break;
    default:
        log_fatal("handle ee slow byte write %08x = %02x", addr, data);
    }
}

void Memory::EEWriteHalf(u32 addr, u16 data) {
    switch (addr) {
    case 0x1A000008:
    case 0x1F801470:
    case 0x1F801472:
        // what is this
        break;
    default:
        log_fatal("handle ee slow half write %08x = %04x", addr, data);
    }
}

void Memory::EEWriteWord(u32 addr, u32 data) {
    if (in_range(0x10000000, 0x10002000, addr)) {
        system->timers.WriteChannel(addr, data);
        return;
    }

    if ((addr >= EE_DMA_REGION1_START && addr < EE_DMA_REGION1_END) ||
        (addr >= EE_DMA_REGION2_START && addr < EE_DMA_REGION2_END)) {
        return system->dmac.WriteRegister(addr, data);
    } 

    switch (addr) {
    case 0x10002000:
        system->ipu.WriteCommand(data);
        break;
    case 0x10002010:
        system->ipu.WriteControl(data);
        break;
    case 0x10003000:
        system->gif.WriteCTRL(data);
        break;
    case 0x10003810:
        system->vif0.WriteFBRST(data);
        break;
    case 0x10003820:
        system->vif0.WriteERR(data);
        break;
    case 0x10003830:
        system->vif0.WriteMark(data);
        break;
    case 0x10003C00:
        system->vif1.WriteStat(data);
        break;
    case 0x10003C10:
        system->vif1.WriteFBRST(data);
        break;
    case 0x1000F000:
        system->ee_intc.WriteStat(data);
        break;
    case 0x1000F010:
        system->ee_intc.WriteMask(data);
        break;
    case 0x1000F200:
        system->sif.WriteMSCOM(data);
        break;
    case 0x1000F220:
        system->sif.SetMSFLAG(data);
        break;
    case 0x1000F230:
        system->sif.SetSMFLAG(data);
        break;
    case 0x1000F240:
        system->sif.WriteEEControl(data);
        break;
    case 0x1000F260:
        system->sif.WriteBD6(data);
        break;
    case 0x1000F100: 
    case 0x1000F120: 
    case 0x1000F140:
    case 0x1000F150:
    case 0x1000F400:
    case 0x1000F410:
    case 0x1000F420:
    case 0x1000F450:
    case 0x1000F460:
    case 0x1000F480:
    case 0x1000F490:
    case 0x1000F500:
    case 0x1000F510:
        // undocumented
        break;
    case 0x1000F430:
        if ((((data >> 16) & 0xFFF) == 0x21) && (((data >> 6) & 0xF) == 1) && (((mch_drd >> 7) & 1) == 0)) {
            rdram_sdevid = 0;
        }
                
        mch_ricm = data & ~0x80000000;
        break;
    case 0x1000F440:
        mch_drd = data;
        break;
    case 0x1F80141C:
        // what is this
        break;
    default:
        log_fatal("handle ee slow word write %08x = %08x", addr, data);
    }
}

void Memory::EEWriteDouble(u32 addr, u64 data) {
    if (in_range(0x11008000, 0x1100C000, addr)) {
        system->vu1.WriteCodeMemory<u64>(addr, data);
        return;
    }

    if (in_range(0x10000000, 0x10002000, addr)) {
        system->timers.WriteChannel(addr, data);
        return;
    }

    switch (addr) {
    case 0x12000010:
        system->gs.WriteSMODE1(data);
        break;
    case 0x12000020:
        system->gs.WriteSMODE2(data);
        break;
    case 0x12000030:
        system->gs.WriteSRFSH(data);
        break;
    case 0x12000040:
        system->gs.WriteSYNCH1(data);
        break;
    case 0x12000050:
        system->gs.WriteSYNCH2(data);
        break;
    case 0x12000060:
        system->gs.WriteSYNCV(data);
        break;
    case 0x12001000:
        system->gs.WriteCSR(data);
        break;
    default:
        log_fatal("handle ee slow double write %08x = %016lx", addr, data);
    }
}

void Memory::EEWriteQuad(u32 addr, u128 data) {
    if (in_range(0x11000000, 0x11001000, addr)) {
        system->vu0.WriteCodeMemory<u128>(addr & 0xFFF, data);
        return;
    }

    if (in_range(0x11004000, 0x11005000, addr)) {
        system->vu0.WriteDataMemory<u128>(addr & 0xFFF, data);
        return;
    }

    if (in_range(0x1100C000, 0x11010000, addr)) {
        system->vu1.WriteDataMemory<u128>(addr, data);
        return;
    }

    switch (addr) {
    case 0x10004000:
    case 0x10005000:
        // handle vif stuff later
        break;
    case 0x10006000:
        system->gif.WriteFIFO(data);
        break;
    case 0x10007010:
        // handle ipu fifo later
        break;
    default:
        log_fatal("handle ee slow quad write %08x = %016lx%016lx", addr, data.i.hi, data.i.lo);
    }
}


template u8 Memory::IOPRead(VAddr vaddr);
template u16 Memory::IOPRead(VAddr vaddr);
template u32 Memory::IOPRead(VAddr vaddr);
template <typename T>
T Memory::IOPRead(VAddr vaddr) {
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
    switch (addr) {
    case 0x1F402005:
        // cdvd n command status
        return 0;
    default:
        log_fatal("handle slow byte read %08x", addr);
    }

    return 0;
}

u16 Memory::IOPReadHalf(u32 addr) {
    switch (addr) {
    default:
        log_fatal("handle slow half read %08x", addr);
    }

    return 0;
}

u32 Memory::IOPReadWord(u32 addr) {
    if ((addr >= IOP_DMA_REGION1_START && addr < IOP_DMA_REGION1_END) ||
        (addr >= IOP_DMA_REGION2_START && addr < IOP_DMA_REGION2_END)) {
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
        log_fatal("[Memory] unhandled iop word read %08x", addr);
    }

    return 0;
}

template void Memory::IOPWrite(VAddr vaddr, u8 data);
template void Memory::IOPWrite(VAddr vaddr, u16 data);
template void Memory::IOPWrite(VAddr vaddr, u32 data);
template <typename T>
void Memory::IOPWrite(VAddr vaddr, T data) {
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
        log_fatal("[Memory] unhandled iop byte write %08x = %02x", addr, data);
        break;
    }
}

void Memory::IOPWriteHalf(u32 addr, u16 data) {
    if ((addr >= IOP_DMA_REGION1_START && addr < IOP_DMA_REGION1_END) ||
        (addr >= IOP_DMA_REGION2_START && addr < IOP_DMA_REGION2_END)) {
        system->iop_dmac.WriteRegister(addr, data);
        return;
    } else if ((addr >= IOP_TIMERS_REGION1_START && addr < IOP_TIMERS_REGION1_END) ||
        (addr >= IOP_TIMERS_REGION2_START && addr < IOP_TIMERS_REGION2_END)) {
        system->iop_timers.WriteRegister(addr, data);
        return;
    }

    switch (addr) {
    default:
        log_fatal("handle slow half write %08x = %04x", addr, data);
    }
}

void Memory::IOPWriteWord(u32 addr, u32 data) {
    if ((addr >= IOP_DMA_REGION1_START && addr < IOP_DMA_REGION1_END) ||
        (addr >= IOP_DMA_REGION2_START && addr < IOP_DMA_REGION2_END)) {
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
        // not sure what this is
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
        log_fatal("[Memory] unhandled iop word write %08x = %08x", addr, data);
    }
}