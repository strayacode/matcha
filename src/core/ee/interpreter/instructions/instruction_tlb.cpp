#include <common/types.h>
#include <core/ee/interpreter/interpreter.h>
#include <core/system.h>

void EEInterpreter::tlbwi() {
    // when we handle mmu emulation the tlb will be used
}

void EEInterpreter::di() {
    u32 status = system->ee_cop0.GetReg(12);
    bool edi = status & (1 << 17);
    bool exl = status & (1 << 1);
    bool erl = status & (1 << 2);
    bool ksu = ((status >> 3) & 0x3) == 0;

    if (edi || exl || erl || ksu) {
        system->ee_cop0.SetReg(12, status & ~(1 << 16));
    } 
}

void EEInterpreter::eret() {
    u32 status = system->ee_cop0.GetReg(12);
    bool erl = status & (1 << 2);
    u32 errorepc = system->ee_cop0.GetReg(30);
    u32 epc = system->ee_cop0.GetReg(14);

    log_warn("eret pc %08x", regs.pc);

    if (erl) {
        regs.pc = errorepc - 4;
        system->ee_cop0.SetReg(12, status & ~(1 << 2));
    } else {
        regs.pc = epc - 4;
        system->ee_cop0.SetReg(12, status & ~(1 << 1));
    }
}

void EEInterpreter::ei() {
    u32 status = system->ee_cop0.GetReg(12);
    bool edi = status & (1 << 17);
    bool exl = status & (1 << 1);
    bool erl = status & (1 << 2);
    bool ksu = ((status >> 3) & 0x3) == 0;

    if (edi || exl || erl || ksu) {
        system->ee_cop0.SetReg(12, status | (1 << 16));
    } 
}