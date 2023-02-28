#pragma once

#include <array>
#include "common/types.h"
#include "core/iop/instruction.h"
#include "core/iop/executor.h"

namespace iop {

struct Context;

class Interpreter : public Executor {
public:
    Interpreter(Context& ctx);

    void Reset() override;
    void Run(int cycles) override;

    enum class InstructionTable {
        Primary,
        Secondary,
        RegImm,
        COP0,
    };

    void DoException(Exception exception);
    void RaiseInterrupt(bool value);
    void CheckInterrupts();

private:
    typedef void (Interpreter::*InstructionHandler)();
    void RegisterOpcode(InstructionHandler handler, int index, InstructionTable table);

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
    void bcondz();
    void xorr();
    void sllv();
    void mfhi();
    void multu();
    void mthi();
    void mtlo();
    void syscall_exception();
    void rfe();
    void mult();
    void nor();
    void srlv();
    void add();
    void div();
    void lwl();
    void lwr();
    void swl();
    void swr();
    void srav();

    void IOPPuts();

private:
    bool branch_delay;
    bool branch;

    Instruction inst;
    Context& ctx;

    std::array<InstructionHandler, 64> primary_table;
    std::array<InstructionHandler, 64> secondary_table;
};

} // namespace iop