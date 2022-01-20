#include <assert.h>
#include <array>
#include "core/ee/ee_core.h"
#include "core/system.h"
#include "core/ee/disassembler.h"

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

EECore::EECore(System& system) : system(system) {}

void EECore::Reset() {
    for (int i = 0; i < 512; i++) {
        gpr[i] = 0;
    }

    pc = 0xBFC00000;
    next_pc = 0;
    hi = 0;
    lo = 0;
    hi1 = 0;
    lo1 = 0;
    sa = 0;
    inst.data = 0;
    branch_delay = false;
    branch = false;

    cop0.Reset();
    cop1.Reset();
    interpreter_table.Generate();
}

bool disassemble = false;

void EECore::Run(int cycles) {
    while (cycles--) {
        inst = CPUInstruction{ReadWord(pc)};

        // if (pc == 0x82000) {
        //     log_fatal("pk");
        // }

        interpreter_table.Execute(*this, inst);

        pc += 4;

        if (branch_delay) {
            if (branch) {
                pc = next_pc;
                branch_delay = false;
                branch = false;
            } else {
                branch = true;
            }
        }

        cop0.CountUp();
        CheckInterrupts();
    }
}

u8 EECore::ReadByte(u32 addr) {
    return system.memory.EERead<u8>(addr);
}

u16 EECore::ReadHalf(u32 addr) {
    return system.memory.EERead<u16>(addr);
}

u32 EECore::ReadWord(u32 addr) {
    return system.memory.EERead<u32>(addr);
}

u64 EECore::ReadDouble(u32 addr) {
    return system.memory.EERead<u64>(addr);
}

u128 EECore::ReadQuad(u32 addr) {
    u128 data;
    // log_debug("low double at address %08x = %016lx", addr, system.memory.EERead<u64>(addr));
    // log_debug("high double at address %08x = %016lx", addr + 8, system.memory.EERead<u64>(addr + 8));
    data.i.lo = system.memory.EERead<u64>(addr);
    data.i.hi = system.memory.EERead<u64>(addr + 8);

    return data;
}

void EECore::WriteByte(u32 addr, u8 data) {
    system.memory.EEWrite<u8>(addr, data);
}

void EECore::WriteHalf(u32 addr, u16 data) {
    system.memory.EEWrite<u16>(addr, data);
}

void EECore::WriteWord(u32 addr, u32 data) {
    system.memory.EEWrite<u32>(addr, data);
}

void EECore::WriteDouble(u32 addr, u64 data) {
    system.memory.EEWrite<u64>(addr, data);
}

void EECore::WriteQuad(u32 addr, u128 data) {
    system.memory.EEWrite<u128>(addr, data);
}

void EECore::DoException(u32 target, ExceptionType exception) {
    u32 status = cop0.GetReg(12);
    u32 cause = cop0.GetReg(13);

    bool level2_exception = static_cast<int>(exception) >= 14;
    int code = level2_exception ? static_cast<int>(exception) - 14 : static_cast<int>(exception);

    if (level2_exception) {
        log_fatal("handle level 2 exception");
    } else {
        cause |= (code << 2);
        if (branch_delay) {
            cop0.SetReg(14, pc - 4);
            cause |= (1 << 31);
        } else {
            cop0.SetReg(14, pc);
            cause &= ~(1 << 31);
        }

        status |= (1 << 1);
        pc = target - 4;
    }

    cop0.SetReg(12, status);
    cop0.gpr[13] = cause;
}

void EECore::SendInterruptSignal(int signal, bool value) {
    if (value) {
        cop0.gpr[13] |= (1 << (10 + signal));
    } else {
        cop0.gpr[13] &= ~(1 << (10 + signal));
    }
}

void EECore::CheckInterrupts() {
    if (InterruptsEnabled()) {
        bool int0_enable = (cop0.gpr[12] >> 10) & 0x1;
        bool int0_pending = (cop0.gpr[13] >> 10) & 0x1;
        bool timer_enable = (cop0.gpr[12] >> 15) & 0x1;

        assert(timer_enable == false);
        
        if (int0_enable && int0_pending) {
            DoException(0x80000200, ExceptionType::Interrupt);
            return;
        }

        bool int1_enable = (cop0.gpr[12] >> 11) & 0x1;
        bool int1_pending = (cop0.gpr[13] >> 11) & 0x1;

        if (int1_enable && int1_pending) {
            DoException(0x80000200, ExceptionType::Interrupt);
            return;
        }

        // TODO: handle cop0 compare interrupts
    }
}

bool EECore::InterruptsEnabled() {
    bool ie = cop0.gpr[12] & 0x1;
    bool eie = (cop0.gpr[12] >> 16) & 0x1;
    bool exl = (cop0.gpr[12] >> 1) & 0x1;
    bool erl = (cop0.gpr[12] >> 2) & 0x1;

    return ie && eie && !exl && !erl;
}

void EECore::PrintRegs() {
    for (int i = 0; i < 32; i++) {
        log_debug("%s: %016lx%016lx", EEGetRegisterName(i).c_str(), GetReg<u128>(i).i.hi, GetReg<u128>(i).i.lo);
    }
}

std::string EECore::GetSyscallInfo(int index) {
    index = (s8)(u8)index;

    if (index < 0) {
        index = -index;
    }

    return syscall_info[index];
}