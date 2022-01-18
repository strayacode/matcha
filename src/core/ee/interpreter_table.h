#pragma once

#include <array>
#include "common/cpu_types.h"

class EECore;

using InterpreterInstruction = void (*)(EECore& cpu, CPUInstruction inst);

class InterpreterTable {
public:
    void Generate();
    InterpreterInstruction GetInterpreterInstruction(EECore& cpu, CPUInstruction inst);
    void Execute(EECore& cpu, CPUInstruction inst);
    void RegisterOpcode(InterpreterInstruction handler, int index, InstructionTable table);

private:
    std::array<InterpreterInstruction, 64> primary_table;
    std::array<InterpreterInstruction, 64> secondary_table;
    std::array<InterpreterInstruction, 32> regimm_table;
    std::array<InterpreterInstruction, 32> cop0_table;
    std::array<InterpreterInstruction, 32> cop1_table;
    std::array<InterpreterInstruction, 64> fpu_s_table;
    std::array<InterpreterInstruction, 32> cop2_table;
    std::array<InterpreterInstruction, 64> tlb_table;
    std::array<InterpreterInstruction, 64> mmi_table;
    std::array<InterpreterInstruction, 32> mmi1_table;
    std::array<InterpreterInstruction, 32> mmi2_table;
    std::array<InterpreterInstruction, 32> mmi3_table;
};