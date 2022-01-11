#pragma once

#include <common/types.h>
#include <common/log.h>
#include <common/log_file.h>
#include <common/arithmetic.h>
#include <core/iop/cpu_core.h>
#include <common/cpu_types.h>
#include <array>

class System;

// TODO: just have the interpreter functions in a namespace

class IOPInterpreter : public IOPCore {
public:
    IOPInterpreter(System* system);

    void Reset() override;
    void Run(int cycles) override;

private:
    typedef void (IOPInterpreter::*InstructionHandler)();
    void RegisterOpcode(InstructionHandler handler, int index, InstructionTable table);

    u8 ReadByte(u32 addr);
    u16 ReadHalf(u32 addr);
    u32 ReadWord(u32 addr);

    void WriteByte(u32 addr, u8 data);
    void WriteHalf(u32 addr, u16 data);
    void WriteWord(u32 addr, u32 data);

    void UndefinedInstruction();
    void SecondaryInstruction();
    void COP0Instruction();

    void mfc0();
    void sll();
    void slti();
    void bne();
    void lui();
    void ori();
    void jr();
    void beq();
    void lw();
    void andi();
    void addiu();
    void addi();
    void orr();
    void sw();
    void sb();
    void mtc0();
    void lb();
    void jal();
    void addu();
    void sltu();
    void lh();
    void andd();
    void slt();
    void j();
    void lbu();
    void sra();
    void sltiu();
    void lhu();
    void srl();
    void subu();
    void blez();
    void bgtz();
    void divu();
    void mflo();
    void sh();
    void jalr();

    CPUInstruction inst;

    std::array<InstructionHandler, 64> primary_table;
    std::array<InstructionHandler, 64> secondary_table;

    bool branch_delay;
    bool branch;
};