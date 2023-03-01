#include "common/log.h"
#include "common/memory.h"
#include "core/iop/context.h"
#include "core/system.h"

namespace iop {

Context::Context(System& system) : dmac(system), timers(system), intc(*this), sio2(intc), interpreter(*this), system(system) {}

void Context::Reset() {
    gpr.fill(0);
    pc = 0xbfc00000;
    npc = 0;
    hi = 0;
    lo = 0;
    
    cop0.Reset();
    cdvd.Reset();
    dmac.Reset();
    timers.Reset();
    intc.Reset();
    sio2.Reset();
    interpreter.Reset();

    // do initial hardcoded mappings
    vtlb.Reset();
    vtlb.Map(system.iop_ram->data(), 0x00000000, 0x200000);
    vtlb.Map(system.iop_ram->data(), 0x80000000, 0x200000);
    vtlb.Map(system.bios->data(), 0x9fc00000, 0x400000);
    vtlb.Map(system.iop_ram->data(), 0xa0000000, 0x200000);
    vtlb.Map(system.bios->data(), 0xbfc00000, 0x400000);
}

void Context::Run(int cycles) {
    interpreter.Run(cycles);
    dmac.Run(cycles);
    timers.Run(cycles);
}

template u8 Context::Read(VirtualAddress vaddr);
template u16 Context::Read(VirtualAddress vaddr);
template u32 Context::Read(VirtualAddress vaddr);
template <typename T>
T Context::Read(VirtualAddress vaddr) {
    auto pointer = vtlb.Lookup<T>(vaddr);
    if (pointer) {
        return common::Read<T>(pointer);
    } else {
        return ReadIO(vaddr & 0x1fffffff);
    }
}

template void Context::Write(VirtualAddress vaddr, u8 value);
template void Context::Write(VirtualAddress vaddr, u16 value);
template void Context::Write(VirtualAddress vaddr, u32 value);
template <typename T>
void Context::Write(VirtualAddress vaddr, T value) {
    auto pointer = vtlb.Lookup<T>(vaddr);
    if (pointer) {
        return common::Write<T>(pointer, value);
    } else {
        WriteIO(vaddr & 0x1fffffff, value);
    }
}

void Context::RaiseInterrupt(bool value) {
    interpreter.RaiseInterrupt(value);
    if (value) {
        cop0.cause.data |= 1 << 10;
    } else {
        cop0.cause.data &= ~(1 << 10);
    }
}

u32 Context::ReadIO(u32 paddr) {
    if (paddr >= 0x1f402004 && paddr < 0x1f402019) {
        return cdvd.ReadRegister(paddr);
    } else if (paddr >= 0x1f801070 && paddr < 0x1f801079) {
        return intc.ReadRegister(paddr);
    } else if (paddr >= 0x1f801080 && paddr < 0x1f801100) {
        return dmac.ReadRegister(paddr);
    } else if (paddr >= 0x1f801100 && paddr < 0x1f801130) {
        return timers.ReadRegister(paddr);
    } else if (paddr >= 0x1f801480 && paddr < 0x1f8014b0) {
        return timers.ReadRegister(paddr);
    } else if (paddr >= 0x1f801500 && paddr < 0x1f80155f) {
        return dmac.ReadRegister(paddr);
    } else if (paddr >= 0x1f801570 && paddr < 0x1f80157f) {
        return dmac.ReadRegister(paddr);
    } else if ((paddr >> 24) == 0x1e) {
        // not sure what this is
        return 0;
    } else if (paddr >= 0x1f900000 && paddr < 0x1f900400) {
        return 0;
    } else if (paddr >= 0x1f900760 && paddr < 0x1f900770) {
        return 0;
    } else if (paddr >= 0x1f900400 && paddr < 0x1f900800) {
        return system.spu2.ReadRegister(paddr);
    } else if (paddr >= 0x1f808200 && paddr < 0x1f808278) {
        return sio2.ReadRegister(paddr);
    }

    switch (paddr) {
    case 0x1f80100c:
    case 0x1f801400:
    case 0x1f801010:
    case 0x1f801450:
    case 0x1ffe0130:
    case 0x1f801414:
        return 0;
    case 0x1d000010:
        return system.sif.ReadSMCOM();
    case 0x1d000020:
        return system.sif.ReadMSFLAG();
    case 0x1d000030:
        return system.sif.ReadSMFLAG();
    case 0x1d000040:
        return system.sif.ReadControl();
    case 0x1d000060:
        return system.sif.bd6;
    default:
        common::Log("[iop::Context] handle io read %08x", paddr);
    }

    return 0;
}

void Context::WriteIO(u32 paddr, u32 value) {
    if (paddr >= 0x1f801080 && paddr < 0x1f801100) {
        dmac.WriteRegister(paddr, value);
        return;
    }else if (paddr >= 0x1f801100 && paddr < 0x1f801130) {
        timers.WriteRegister(paddr, value);
        return;
    } else if (paddr >= 0x1f801480 && paddr < 0x1f8014b0) {
        timers.WriteRegister(paddr, value);
        return;
    } else if (paddr >= 0x1f801500 && paddr < 0x1f80155f) {
        dmac.WriteRegister(paddr, value);
        return;
    } else if (paddr >= 0x1f801570 && paddr < 0x1f80157f) {
        dmac.WriteRegister(paddr, value);
        return;
    } else if (paddr >= 0x1f801070 && paddr < 0x1f801079) {
        intc.WriteRegister(paddr, value);
        return;
    } else if (paddr >= 0x1f900000 && paddr < 0x1f900400) {
        system.spu.WriteRegister(paddr, value);
        return;
    } else if (paddr >= 0x1f900760 && paddr < 0x1f900770) {
        system.spu.WriteRegister(paddr, value);
        return;
    } else if (paddr >= 0x1f900400 && paddr < 0x1f900800) {
        system.spu2.WriteRegister(paddr, value);
        return;
    } else if (paddr >= 0x1f808200 && paddr < 0x1f808278) {
        sio2.WriteRegister(paddr, value);
        return;
    }

    switch (paddr) {
    case 0x1f801010:
        // sif2/gpu ssbus
        break;
    case 0x1f801450:
        // random config register
        break;
    case 0x1f801004:
    case 0x1f80100c:
    case 0x1f801014:
    case 0x1f801018:
    case 0x1f80101c:
    case 0x1f801020:
    case 0x1f801400:
    case 0x1f801404:
    case 0x1f801408:
    case 0x1f80140c:
    case 0x1f801410:
    case 0x1f801414:
    case 0x1f801418:
    case 0x1f80141c:
    case 0x1f801420:
    case 0x1f802070:
    case 0x1f801060:
    case 0x1f801560:
    case 0x1f801564:
    case 0x1f801568:
    case 0x1ffe0130:
    case 0x1ffe0140:
    case 0x1ffe0144:
    case 0x1f8015f0:
        // undocumented
        break;
    case 0x1d000010:
        system.sif.WriteSMCOM(value);
        break;
    case 0x1d000020:
        system.sif.ResetMSFLAG(value);
        break;
    case 0x1d000030:
        system.sif.SetSMFLAG(value);
        break;
    case 0x1d000040:
        system.sif.WriteIOPControl(value);
        break;
    case 0x1f900b60:
    case 0x1f900b62:
        // are these spu related?
        break;
    default:
        common::Log("[iop::Context] handle io write %08x = %08x", paddr, value);
    }
}

} // namespace iop