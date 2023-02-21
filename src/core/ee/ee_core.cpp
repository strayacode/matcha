#include <assert.h>
#include <array>
#include "common/log_file.h"
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

void EECore::Run(int cycles) {
    while (cycles--) {
        inst = CPUInstruction{ReadWord(pc)};
        common::debug("run instruction %08x", inst.data);

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
    return system.memory.EEReadByte(addr);
}

u16 EECore::ReadHalf(u32 addr) {
    return system.memory.EEReadHalf(addr);
}

u32 EECore::ReadWord(u32 addr) {
    return system.memory.EEReadWord(addr);
}

u64 EECore::ReadDouble(u32 addr) {
    return system.memory.EEReadDouble(addr);
}

u128 EECore::ReadQuad(u32 addr) {
    u128 data;
    // common::debug("low double at address %08x = %016lx", addr, system.memory.EERead<u64>(addr));
    // common::debug("high double at address %08x = %016lx", addr + 8, system.memory.EERead<u64>(addr + 8));
    data.i.lo = system.memory.EEReadDouble(addr);
    data.i.hi = system.memory.EEReadDouble(addr + 8);

    return data;
}

void EECore::WriteByte(u32 addr, u8 data) {
    system.memory.EEWriteByte(addr, data);
}

void EECore::WriteHalf(u32 addr, u16 data) {
    system.memory.EEWriteHalf(addr, data);
}

void EECore::WriteWord(u32 addr, u32 data) {
    system.memory.EEWriteWord(addr, data);
}

void EECore::WriteDouble(u32 addr, u64 data) {
    system.memory.EEWriteDouble(addr, data);
}

void EECore::WriteQuad(u32 addr, u128 data) {
    system.memory.EEWriteQuad(addr, data);
}

void EECore::DoException(u32 target, ExceptionType exception) {
    LogFile::Get().Log("[EE] trigger exception with type %02x at pc = %08x\n", static_cast<int>(exception), pc);

    bool level2_exception = static_cast<int>(exception) >= 14;
    int code = level2_exception ? static_cast<int>(exception) - 14 : static_cast<int>(exception);

    if (level2_exception) {
        common::error("handle level 2 exception");
    } else {
        cop0.cause.exception = code;
        cop0.gpr[14] = pc - 4 * branch_delay;
        cop0.cause.bd = branch_delay;
        cop0.gpr[12] |= (1 << 1);

        pc = target - 4;
    }

    branch_delay = false;
    branch = false;
}

void EECore::SendInterruptSignal(int signal, bool value) {
    if (signal == 0) {
        cop0.cause.int0_pending = value;
    } else {
        // int1 signal
        cop0.cause.int1_pending = value;
    }
}

void EECore::CheckInterrupts() {
    if (InterruptsEnabled()) {
        bool int0_enable = (cop0.gpr[12] >> 10) & 0x1;
        bool timer_enable = (cop0.gpr[12] >> 15) & 0x1;

        assert(timer_enable == false);
        
        if (int0_enable && cop0.cause.int0_pending) {
            LogFile::Get().Log("[EE] do int0 interrupt\n");
            DoException(0x80000200, ExceptionType::Interrupt);
            return;
        }

        bool int1_enable = (cop0.gpr[12] >> 11) & 0x1;
        
        if (int1_enable && cop0.cause.int1_pending) {
            LogFile::Get().Log("[EE] do int1 interrupt\n");
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

void EECore::PrintState() {
    LogFile::Get().Log("[EE State]\n");
    for (int i = 0; i < 32; i++) {
        LogFile::Get().Log("%s: %016lx%016lx\n", EEGetRegisterName(i).c_str(), GetReg<u128>(i).i.hi, GetReg<u128>(i).i.lo);
    }

    LogFile::Get().Log("pc: %08x npc: %08x\n", pc, next_pc);
    LogFile::Get().Log("branch: %d branch delay: %d\n", branch, branch_delay);
    LogFile::Get().Log("%s\n", EEDisassembleInstruction(inst, pc).c_str());
}

std::string EECore::GetSyscallInfo(int index) {
    index = (s8)(u8)index;

    if (index < 0) {
        index = -index;
    }

    return syscall_info[index];
}

void EECore::LogInstruction() {
    LogFile::Get().Log("[EE] %08x %08x %s\n", pc, inst.data, EEDisassembleInstruction(inst, pc).c_str());
}