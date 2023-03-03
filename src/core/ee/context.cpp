#include <assert.h>
#include <array>
#include "common/log.h"
#include "common/memory.h"
#include "core/ee/context.h"
#include "core/ee/disassembler.h"
#include "core/system.h"

namespace ee {

static std::array<std::string, 256> syscall_info = {
    "RFU000_FullReset", "ResetEE", "SetGsCrt", "RFU003",
    "Exit",	"RFU005", "LoadExecPS2", "ExecPS2",
    "RFU008", "RFU009",	"AddSbusIntcHandler", "RemoveSbusIntcHandler",
    "Interrupt2Iop", "SetVTLBRefillHandler", "SetVCommonHandler", "SetVInterruptHandler",
    "AddIntcHandler", "RemoveIntcHandler", "AddDmacHandler", "RemoveDmacHandler",
    "_EnableIntc", "_DisableIntc", "_EnableDmac", "_DisableDmac",
    "_SetAlarm", "_ReleaseAlarm", "_iEnableIntc", "_iDisableIntc",
    "_iEnableDmac", "_iDisableDmac", "_iSetAlarm", "_iReleaseAlarm",
    "CreateThread", "DeleteThread", "StartThread", "ExitThread",
    "ExitDeleteThread", "TerminateThread", "iTerminateThread", "DisableDispatchThread",
    "EnableDispatchThread", "ChangeThreadPriority", "iChangeThreadPriority", "RotateThreadReadyQueue",
    "iRotateThreadReadyQueue", "ReleaseWaitThread",	"iReleaseWaitThread", "GetThreadId",
    "ReferThreadStatus", "iReferThreadStatus", "SleepThread", "WakeupThread",
    "_iWakeupThread", "CancelWakeupThread",	"iCancelWakeupThread", "SuspendThread",
    "iSuspendThread", "ResumeThread", "iResumeThread", "JoinThread",
    "RFU060", "RFU061", "EndOfHeap", "RFU063",
    "CreateSema", "DeleteSema", "SignalSema", "iSignalSema",
    "WaitSema",  "PollSema", "iPollSema", "ReferSemaStatus",
    "iReferSemaStatus", "RFU073", "SetOsdConfigParam", "GetOsdConfigParam",
    "GetGsHParam", "GetGsVParam", "SetGsHParam", "SetGsVParam",
    "RFU080_CreateEventFlag", "RFU081_DeleteEventFlag",
    "RFU082_SetEventFlag", "RFU083_iSetEventFlag",
    "RFU084_ClearEventFlag", "RFU085_iClearEventFlag",
    "RFU086_WaitEventFlag", "RFU087_PollEventFlag",
    "RFU088_iPollEventFlag", "RFU089_ReferEventFlagStatus",
    "RFU090_iReferEventFlagStatus", "RFU091_GetEntryAddress",
    "EnableIntcHandler_iEnableIntcHandler",
    "DisableIntcHandler_iDisableIntcHandler",
    "EnableDmacHandler_iEnableDmacHandler",
    "DisableDmacHandler_iDisableDmacHandler",
    "KSeg0", "EnableCache", "DisableCache", "GetCop0",
    "FlushCache", "RFU101", "CpuConfig", "iGetCop0",
    "iFlushCache", "RFU105", "iCpuConfig", "sceSifStopDma",
    "SetCPUTimerHandler", "SetCPUTimer", "SetOsdConfigParam2", "SetOsdConfigParam2",
    "GsGetIMR_iGsGetIMR", "GsGetIMR_iGsPutIMR", "SetPgifHandler", "SetVSyncFlag",
    "RFU116", "print", "sceSifDmaStat", "sceSifSetDma",
    "sceSifSetDChain", "sceSifSetReg", "sceSifGetReg", "ExecOSD",
    "Deci2Call", "PSMode", "MachineType", "GetMemorySize",
};

Context::Context(System& system) : dmac(system), timers(intc), intc(*this), system(system), interpreter(*this) {
    rdram = std::make_unique<std::array<u8, 0x2000000>>();
}

void Context::Reset() {
    gpr.fill(0);
    pc = 0xbfc00000;
    npc = 0;
    hi = 0;
    lo = 0;
    hi1 = 0;
    lo1 = 0;
    sa = 0;
    
    scratchpad.fill(0);
    rdram->fill(0);

    mch_drd = 0;
    rdram_sdevid = 0;
    mch_ricm = 0;

    cop0.Reset();
    cop1.Reset();
    dmac.Reset();
    timers.Reset();
    intc.Reset();
    interpreter.Reset();

    // do initial hardcoded mappings
    vtlb.Reset();
    vtlb.Map(rdram->data(), 0x00000000, 0x2000000, 0x1ffffff);
    vtlb.Map(rdram->data(), 0x20000000, 0x2000000, 0x1ffffff);
    vtlb.Map(rdram->data(), 0x30100000, 0x2000000, 0x1ffffff);
    vtlb.Map(scratchpad.data(), 0x70000000, 0x4000, 0x3fff);
    vtlb.Map(rdram->data(), 0x80000000, 0x2000000, 0x1ffffff);
    vtlb.Map(system.bios->data(), 0x9fc00000, 0x400000, 0x3fffff);
    vtlb.Map(rdram->data(), 0xa0000000, 0x2000000, 0x1ffffff);
    vtlb.Map(system.bios->data(), 0xbfc00000, 0x400000, 0x3fffff);

    // deci2call tlb region which gets mapped in the bios
    // later when we handle the tlb we can remove this mapping
    vtlb.Map(rdram->data(), 0xffff8000, 0x8000, 0x7ffff);
}

void Context::Run(int cycles) {
    interpreter.Run(cycles);

    // timers and dmac run at half the speed of the ee (bus speed)
    timers.Run(cycles / 2);
    dmac.Run(cycles / 2);
}

template u8 Context::Read(VirtualAddress vaddr);
template u16 Context::Read(VirtualAddress vaddr);
template u32 Context::Read(VirtualAddress vaddr);
template u64 Context::Read(VirtualAddress vaddr);
template <typename T>
T Context::Read(VirtualAddress vaddr) {
    auto pointer = vtlb.Lookup<T>(vaddr);
    if (pointer) {
        return common::Read<T>(pointer);
    } else {
        return ReadIO(vaddr & 0x1fffffff);
    }
}

template <>
u128 Context::Read<u128>(VirtualAddress vaddr) {
    u128 value;
    for (int i = 0; i < 4; i++) {
        value.uw[i] = Read<u32>(vaddr + (i * 4));
    }

    return value;
}

template void Context::Write(VirtualAddress vaddr, u8 value);
template void Context::Write(VirtualAddress vaddr, u16 value);
template void Context::Write(VirtualAddress vaddr, u32 value);
template void Context::Write(VirtualAddress vaddr, u64 value);
template <typename T>
void Context::Write(VirtualAddress vaddr, T value) {
    auto pointer = vtlb.Lookup<T>(vaddr);
    if (pointer) {
        return common::Write<T>(pointer, value);
    } else {
        WriteIO(vaddr & 0x1fffffff, value);
    }
}

template <>
void Context::Write<u128>(VirtualAddress vaddr, u128 value) {
    for (int i = 0; i < 4; i++) {
        Write<u32>(vaddr + (i * 4), value.uw[i]);
    }
}

u32 Context::ReadIO(u32 paddr) {
    if (paddr >= 0x10000000 && paddr < 0x10001840) {
        return timers.ReadRegister(paddr);
    } else if (paddr >= 0x10008000 && paddr < 0x1000e000) {
        return dmac.ReadChannel(paddr);
    } else if (paddr >= 0x10003000 && paddr < 0x100030a4) {
        return system.gif.ReadRegister(paddr);
    }

    switch (paddr) {
    case 0x10002010:
        return system.ipu.ReadControl();
    case 0x1000E000:
        return dmac.ReadControl();
    case 0x1000E010:
        return dmac.ReadInterruptStatus();
    case 0x1000E020:
        return dmac.ReadPriorityControl();
    case 0x1000E030:
        return dmac.ReadPriorityControl();
    case 0x1000F000:
        return intc.ReadStat();
    case 0x1000F010:
        return intc.ReadMask();
    case 0x1000F130:
        return 0;
    case 0x1000F200:
        return system.sif.ReadMSCOM();
    case 0x1000F210:
        return system.sif.ReadSMCOM();
    case 0x1000F220:
        return system.sif.ReadMSFLAG();
    case 0x1000F230:
        return system.sif.ReadSMFLAG();
    case 0x1000f430:
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
    case 0x1000f520:
        return dmac.disabled_status;
    default:
        common::Log("[ee::Context] handle io read %08x", paddr);
    }

    return 0;
}

void Context::WriteIO(u32 paddr, u32 value) {
    if (paddr >= 0x10000000 && paddr < 0x10001840) {
        timers.WriteRegister(paddr, value);
        return;
    } if (paddr >= 0x12000000 && paddr < 0x12001084) {
        system.gs.WriteRegisterPrivileged(paddr, value);
        return;
    } else if (paddr >= 0x10008000 && paddr < 0x1000e054) {
        dmac.WriteRegister(paddr, value);
        return;
    } else if (paddr >= 0x1000f520 && paddr < 0x1000f594) {
        dmac.WriteRegister(paddr, value);
        return;
    } else if (paddr >= 0x11000000 && paddr < 0x11001000) {
        system.vu0.WriteCodeMemory(paddr, value);
        return;
    } else if (paddr >= 0x11004000 && paddr < 0x11005000) {
        system.vu0.WriteDataMemory(paddr, value);
        return;
    } else if (paddr >= 0x11008000 && paddr < 0x1100c000) {
        system.vu1.WriteCodeMemory(paddr, value);
        return;
    } else if (paddr >= 0x1100c000 && paddr < 0x11010000) {
        system.vu1.WriteDataMemory(paddr, value);
        return;
    } else if (paddr >= 0x10003000 && paddr < 0x100030a4) {
        system.gif.WriteRegister(paddr, value);
        return;
    } else if (paddr >= 0x10006000 && paddr < 0x10006010) {
        system.gif.WriteRegister(paddr, value);
        return;
    }

    switch (paddr) {
    case 0x10002000:
        system.ipu.WriteCommand(value);
        break;
    case 0x10002010:
        system.ipu.WriteControl(value);
        break;
    case 0x10003810:
        system.vif0.WriteFBRST(value);
        break;
    case 0x10003820:
        system.vif0.WriteERR(value);
        break;
    case 0x10003830:
        system.vif0.WriteMark(value);
        break;
    case 0x10003C00:
        system.vif1.WriteStat(value);
        break;
    case 0x10003C10:
        system.vif1.WriteFBRST(value);
        break;
    case 0x1000F000:
        intc.WriteStat(value);
        break;
    case 0x1000F010:
        intc.WriteMask(value);
        break;
    case 0x1000F180:
        // kputchar
        printf("%c", value);
        break;
    case 0x1000F200:
        system.sif.WriteMSCOM(value);
        break;
    case 0x1000F220:
        system.sif.SetMSFLAG(value);
        break;
    case 0x1000F230:
        system.sif.SetSMFLAG(value);
        break;
    case 0x1000F240:
        system.sif.WriteEEControl(value);
        break;
    case 0x1000F260:
        system.sif.WriteBD6(value);
        break;
    case 0x1000F430:
        if ((((value >> 16) & 0xFFF) == 0x21) && (((value >> 6) & 0xF) == 1) && (((mch_drd >> 7) & 1) == 0)) {
            rdram_sdevid = 0;
        }
                
        mch_ricm = value & ~0x80000000;
        break;
    case 0x1000F440:
        mch_drd = value;
        break;
    default:
        common::Log("[ee::Context] handle io write %08x = %08x", paddr, value);
    }
}

void Context::RaiseInterrupt(int signal, bool value) {
    interpreter.RaiseInterrupt(signal, value);
}

std::string Context::GetSyscallInfo(int index) {
    index = (s8)(u8)index;

    if (index < 0) {
        index = -index;
    }

    return syscall_info[index];
}

} // namespace ee