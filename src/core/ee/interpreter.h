#pragma once

// #include "core/ee/context.h"
#include "core/ee/decoder.h"
#include "core/ee/executor.h"

namespace ee {

struct Context;

struct Interpreter : public Executor {
    Interpreter(Context& ctx);

    void Reset();
    void Run(int cycles);

    void DoException(u32 target, ExceptionType exception);
    void RaiseInterrupt(int signal, bool value);
    void CheckInterrupts();
    bool InterruptsEnabled();
    void PrintState();
    std::string GetSyscallInfo(int index);
    void LogInstruction();

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
    void syscall_exception();
    void bltzl();
    void bgezl();
    void ei();
    void dsubu();
    void plzcw();
    void mtc1();
    void adda_s();
    void ctc1();
    void mfhi1();
    void mfsa();
    void mthi();
    void mthi1();
    void mtlo();
    void mtlo1();
    void mtsa();
    void cfc1();
    void madd_s();
    void lwc1();
    void illegal_instruction();
    void stub_instruction();

private:
    bool branch_delay;
    bool branch;

    Decoder<Interpreter> decoder;
    Instruction inst;
    Context& ctx;
};

} // namespace ee