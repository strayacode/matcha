#include "core/ee/ee_core.h"
#include "core/system.h"

EECore::EECore(System& system) : system(system) {

}

void EECore::Reset() {
    for (int i = 0; i < 512; i++) {
        regs.gpr[i] = 0;
    }

    regs.pc = 0xBFC00000;
    regs.next_pc = 0;
    regs.hi = 0;
    regs.lo = 0;
    regs.hi1 = 0;
    regs.lo1 = 0;
    regs.sa = 0;
    inst.data = 0;
    branch_delay = false;
    branch = false;
}

void EECore::Run(int cycles) {
    log_fatal("handle");
    // while (cycles--) {
    //     inst = CPUInstruction{ReadWord(regs.pc)};

    //     (this->*primary_table[inst.i.opcode])();

    //     regs.pc += 4;

    //     if (branch_delay) {
    //         if (branch) {
    //             regs.pc = regs.next_pc;
    //             branch_delay = false;
    //             branch = false;
    //         } else {
    //             branch = true;
    //         }
    //     }

    //     system->ee_cop0.CountUp();
    // }
}