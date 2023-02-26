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

Context::Context(System& system) : interpreter(*this), system(system) {}

void Context::Reset() {
    gpr.fill(0);
    pc = 0xbfc00000;
    npc = 0;
    hi = 0;
    lo = 0;
    hi1 = 0;
    lo1 = 0;
    sa = 0;
    
    cop0.Reset();
    cop1.Reset();
    interpreter.Reset();
}

void Context::Run(int cycles) {
    interpreter.Run(cycles);
}

template u8 Context::Read(VirtualAddress vaddr);
template u16 Context::Read(VirtualAddress vaddr);
template u32 Context::Read(VirtualAddress vaddr);
template u64 Context::Read(VirtualAddress vaddr);
template u128 Context::Read(VirtualAddress vaddr);
template <typename T>
T Context::Read(VirtualAddress vaddr) {
    auto pointer = vtlb.Lookup(vaddr);
    if (pointer) {
        return common::Read<T>(pointer);
    } else {
        common::Error("[ee::Context] handle unmapped vaddr %08x", vaddr);
    }

    return 0;
}

void Context::WriteByte(u32 addr, u8 data) {
    system.memory.EEWriteByte(addr, data);
}

void Context::WriteHalf(u32 addr, u16 data) {
    system.memory.EEWriteHalf(addr, data);
}

void Context::WriteWord(u32 addr, u32 data) {
    system.memory.EEWriteWord(addr, data);
}

void Context::WriteDouble(u32 addr, u64 data) {
    system.memory.EEWriteDouble(addr, data);
}

void Context::WriteQuad(u32 addr, u128 data) {
    system.memory.EEWriteQuad(addr, data);
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