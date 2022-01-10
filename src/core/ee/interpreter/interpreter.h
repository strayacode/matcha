#pragma once

#include <common/types.h>
#include <common/log.h>
#include <common/log_file.h>
#include <common/arithmetic.h>
#include <common/int128.h>
#include <core/ee/cpu_core.h>
#include <common/cpu_types.h>
#include <core/ee/disassembler.h>
#include <array>

class System;

enum class ExceptionType : int {
    Interrupt = 0,
    TLBModified = 1,
    TLBRefillInstruction = 2,
    TLBRefillStore = 3,
    AddressErrorInstruction = 4,
    AddressErrorStore = 5,
    BusErrorInstruction = 6,
    BusErrorStore = 7,
    Syscall = 8,
    Break = 9,
    Reserved = 10,
    CoprocessorUnusable = 11,
    Overflow = 12,
    Trap = 13,
    Reset = 14,
    NMI = 15,
    PerformanceCounter = 16,
    Debug = 18,
};

class EEInterpreter : public EECore {
public:
    EEInterpreter(System* system);

    void Reset() override;
    void Run(int cycles) override;
private:
    typedef void (EEInterpreter::*InstructionHandler)();
    void RegisterOpcode(InstructionHandler handler, int index, InstructionTable table);

    u8 ReadByte(u32 addr);
    u16 ReadHalf(u32 addr);
    u32 ReadWord(u32 addr);
    u64 ReadDouble(u32 addr);

    void WriteByte(u32 addr, u8 data);
    void WriteHalf(u32 addr, u16 data);
    void WriteWord(u32 addr, u32 data);
    void WriteDouble(u32 addr, u64 data);
    void WriteQuad(u32 addr, u128 data);

    void COP0Instruction();
    void UndefinedInstruction();
    void SecondaryInstruction();
    void TLBInstruction();
    void RegImmInstruction();
    void MMIInstruction();
    void MMI1Instruction();
    void MMI2Instruction();
    void MMI3Instruction();
    void COP2Instruction();
    void COP1Instruction();

    void RecordRegisters();

    void DoException(u32 target, ExceptionType exception);

    void mfc0();
    void sll();
    void slti();
    void bne();
    void lui();
    void ori();
    void jr();
    void mtc0();
    void sync();
    void addiu();
    void sw();
    void tlbwi();
    void jalr();
    void sd();
    void daddu();
    void jal();
    void andi();
    void beq();
    void orr();
    void mult();
    void divu();
    void beql();
    void break_exception();
    void mflo();
    void sltiu();
    void bnel();
    void srl();
    void lb();
    void swc1();
    void lbu();
    void sra();
    void ld();
    void j();
    void lw();
    void sb();
    void blez();
    void slt();
    void addu();
    void sltu();
    void andd();
    void bgez();
    void lhu();
    void movn();
    void subu();
    void bltz();
    void div();
    void mfhi();
    void bgtz();
    void sh();
    void divu1();
    void mflo1();
    void dsrav();
    void dsll32();
    void dsra32();
    void xori();
    void mult1();
    void movz();
    void dsllv();
    void daddiu();
    void sq();
    void lq();
    void lh();
    void por();
    void cache();
    void sllv();
    void dsll();
    void srav();
    void nor();
    void cfc2();
    void ctc2();
    void lwu();
    void ldl();
    void ldr();
    void sdl();
    void sdr();
    void dsrl();
    void srlv();
    void dsrl32();
    void padduw();
    void di();
    void eret();
    void syscall();
    void bltzl();
    void ei();

    CPUInstruction inst;

    std::array<InstructionHandler, 64> primary_table;
    std::array<InstructionHandler, 64> secondary_table;

    bool branch_delay;
    bool branch;

    LogFile log_file;
};