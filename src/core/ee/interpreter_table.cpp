#include "core/ee/interpreter_table.h"
#include "core/ee/ee_interpreter.h"
#include "core/ee/ee_core.h"

void InterpreterTable::Generate() {
    primary_table.fill(&EEInterpreter::unknown_instruction);
    secondary_table.fill(&EEInterpreter::unknown_instruction);
    regimm_table.fill(&EEInterpreter::unknown_instruction);
    cop0_table.fill(&EEInterpreter::unknown_instruction);
    cop1_table.fill(&EEInterpreter::unknown_instruction);
    fpu_s_table.fill(&EEInterpreter::unknown_instruction);
    cop2_table.fill(&EEInterpreter::stub_instruction);
    tlb_table.fill(&EEInterpreter::unknown_instruction);
    mmi_table.fill(&EEInterpreter::unknown_instruction);
    mmi1_table.fill(&EEInterpreter::unknown_instruction);
    mmi2_table.fill(&EEInterpreter::unknown_instruction);
    mmi3_table.fill(&EEInterpreter::unknown_instruction);

    // primary instructions
    RegisterOpcode(&EEInterpreter::j, 2, InstructionTable::Primary);
    RegisterOpcode(&EEInterpreter::jal, 3, InstructionTable::Primary);
    RegisterOpcode(&EEInterpreter::beq, 4, InstructionTable::Primary);
    RegisterOpcode(&EEInterpreter::bne, 5, InstructionTable::Primary);
    RegisterOpcode(&EEInterpreter::blez, 6, InstructionTable::Primary);
    RegisterOpcode(&EEInterpreter::bgtz, 7, InstructionTable::Primary);
    RegisterOpcode(&EEInterpreter::addiu, 9, InstructionTable::Primary);
    RegisterOpcode(&EEInterpreter::slti, 10, InstructionTable::Primary);
    RegisterOpcode(&EEInterpreter::sltiu, 11, InstructionTable::Primary);
    RegisterOpcode(&EEInterpreter::andi, 12, InstructionTable::Primary);
    RegisterOpcode(&EEInterpreter::ori, 13, InstructionTable::Primary);
    RegisterOpcode(&EEInterpreter::xori, 14, InstructionTable::Primary);
    RegisterOpcode(&EEInterpreter::lui, 15, InstructionTable::Primary);
    RegisterOpcode(&EEInterpreter::beql, 20, InstructionTable::Primary);
    RegisterOpcode(&EEInterpreter::bnel, 21, InstructionTable::Primary);
    RegisterOpcode(&EEInterpreter::daddiu, 25, InstructionTable::Primary);
    RegisterOpcode(&EEInterpreter::ldl, 26, InstructionTable::Primary);
    RegisterOpcode(&EEInterpreter::ldr, 27, InstructionTable::Primary);
    RegisterOpcode(&EEInterpreter::lq, 30, InstructionTable::Primary);
    RegisterOpcode(&EEInterpreter::sq, 31, InstructionTable::Primary);
    RegisterOpcode(&EEInterpreter::lb, 32, InstructionTable::Primary);
    RegisterOpcode(&EEInterpreter::lh, 33, InstructionTable::Primary);
    RegisterOpcode(&EEInterpreter::lw, 35, InstructionTable::Primary);
    RegisterOpcode(&EEInterpreter::lbu, 36, InstructionTable::Primary);
    RegisterOpcode(&EEInterpreter::lhu, 37, InstructionTable::Primary);
    RegisterOpcode(&EEInterpreter::lwu, 39, InstructionTable::Primary);
    RegisterOpcode(&EEInterpreter::sb, 40, InstructionTable::Primary);
    RegisterOpcode(&EEInterpreter::sh, 41, InstructionTable::Primary);
    RegisterOpcode(&EEInterpreter::sw, 43, InstructionTable::Primary);
    RegisterOpcode(&EEInterpreter::sdl, 44, InstructionTable::Primary);
    RegisterOpcode(&EEInterpreter::sdr, 45, InstructionTable::Primary);
    RegisterOpcode(&EEInterpreter::cache, 47, InstructionTable::Primary);
    RegisterOpcode(&EEInterpreter::ld, 55, InstructionTable::Primary);
    RegisterOpcode(&EEInterpreter::swc1, 57, InstructionTable::Primary);
    RegisterOpcode(&EEInterpreter::sd, 63, InstructionTable::Primary);

    // secondary instructions
    RegisterOpcode(&EEInterpreter::sll, 0, InstructionTable::Secondary);
    RegisterOpcode(&EEInterpreter::srl, 2, InstructionTable::Secondary);
    RegisterOpcode(&EEInterpreter::sra, 3, InstructionTable::Secondary);
    RegisterOpcode(&EEInterpreter::sllv, 4, InstructionTable::Secondary);
    RegisterOpcode(&EEInterpreter::srlv, 6, InstructionTable::Secondary);
    RegisterOpcode(&EEInterpreter::srav, 7, InstructionTable::Secondary);
    RegisterOpcode(&EEInterpreter::jr, 8, InstructionTable::Secondary);
    RegisterOpcode(&EEInterpreter::jalr, 9, InstructionTable::Secondary);
    RegisterOpcode(&EEInterpreter::movz, 10, InstructionTable::Secondary);
    RegisterOpcode(&EEInterpreter::movn, 11, InstructionTable::Secondary);
    RegisterOpcode(&EEInterpreter::syscall_exception, 12, InstructionTable::Secondary);
    RegisterOpcode(&EEInterpreter::break_exception, 13, InstructionTable::Secondary);
    RegisterOpcode(&EEInterpreter::sync, 15, InstructionTable::Secondary);
    RegisterOpcode(&EEInterpreter::mfhi, 16, InstructionTable::Secondary);
    RegisterOpcode(&EEInterpreter::mflo, 18, InstructionTable::Secondary);
    RegisterOpcode(&EEInterpreter::dsllv, 20, InstructionTable::Secondary);
    RegisterOpcode(&EEInterpreter::dsrav, 23, InstructionTable::Secondary);
    RegisterOpcode(&EEInterpreter::mult, 24, InstructionTable::Secondary);
    RegisterOpcode(&EEInterpreter::div, 26, InstructionTable::Secondary);
    RegisterOpcode(&EEInterpreter::divu, 27, InstructionTable::Secondary);
    RegisterOpcode(&EEInterpreter::addu, 33, InstructionTable::Secondary);
    RegisterOpcode(&EEInterpreter::subu, 35, InstructionTable::Secondary);
    RegisterOpcode(&EEInterpreter::andd, 36, InstructionTable::Secondary);
    RegisterOpcode(&EEInterpreter::orr, 37, InstructionTable::Secondary);
    RegisterOpcode(&EEInterpreter::nor, 39, InstructionTable::Secondary);
    RegisterOpcode(&EEInterpreter::slt, 42, InstructionTable::Secondary);
    RegisterOpcode(&EEInterpreter::sltu, 43, InstructionTable::Secondary);
    RegisterOpcode(&EEInterpreter::daddu, 45, InstructionTable::Secondary);
    RegisterOpcode(&EEInterpreter::dsubu, 47, InstructionTable::Secondary);
    RegisterOpcode(&EEInterpreter::dsll, 56, InstructionTable::Secondary);
    RegisterOpcode(&EEInterpreter::dsrl, 58, InstructionTable::Secondary);
    RegisterOpcode(&EEInterpreter::dsll32, 60, InstructionTable::Secondary);
    RegisterOpcode(&EEInterpreter::dsrl32, 62, InstructionTable::Secondary);
    RegisterOpcode(&EEInterpreter::dsra32, 63, InstructionTable::Secondary);

    // regimm instructions
    RegisterOpcode(&EEInterpreter::bltz, 0, InstructionTable::RegImm);
    RegisterOpcode(&EEInterpreter::bgez, 1, InstructionTable::RegImm);
    RegisterOpcode(&EEInterpreter::bltzl, 2, InstructionTable::RegImm);
    RegisterOpcode(&EEInterpreter::bgezl, 3, InstructionTable::RegImm);

    // cop0 instructions
    RegisterOpcode(&EEInterpreter::mfc0, 0, InstructionTable::COP0);
    RegisterOpcode(&EEInterpreter::mtc0, 4, InstructionTable::COP0);

    // cop1 instructions
    RegisterOpcode(&EEInterpreter::mtc1, 4, InstructionTable::COP1);
    RegisterOpcode(&EEInterpreter::ctc1, 6, InstructionTable::COP1);

    // fpu_s instructions
    RegisterOpcode(&EEInterpreter::adda_s, 24, InstructionTable::FPUS);

    // cop2 instructions
    RegisterOpcode(&EEInterpreter::cfc2, 2, InstructionTable::COP2);
    RegisterOpcode(&EEInterpreter::ctc2, 6, InstructionTable::COP2);

    // tlb instructions
    RegisterOpcode(&EEInterpreter::tlbwi, 2, InstructionTable::TLB);
    RegisterOpcode(&EEInterpreter::eret, 24, InstructionTable::TLB);
    RegisterOpcode(&EEInterpreter::ei, 56, InstructionTable::TLB);
    RegisterOpcode(&EEInterpreter::di, 57, InstructionTable::TLB);

    // mmi instructions
    RegisterOpcode(&EEInterpreter::plzcw, 4, InstructionTable::MMI);
    RegisterOpcode(&EEInterpreter::mflo1, 18, InstructionTable::MMI);
    RegisterOpcode(&EEInterpreter::mult1, 24, InstructionTable::MMI);
    RegisterOpcode(&EEInterpreter::divu1, 27, InstructionTable::MMI);

    // mmi1 instructions
    RegisterOpcode(&EEInterpreter::padduw, 16, InstructionTable::MMI1);

    // mmi3 instructions
    RegisterOpcode(&EEInterpreter::por, 18, InstructionTable::MMI3);
}

InterpreterInstruction InterpreterTable::GetInterpreterInstruction(EECore& cpu, CPUInstruction inst) {
    switch (inst.opcode) {
    case 0:
        return secondary_table[inst.func];
    case 1:
        return regimm_table[inst.rt];
    case 16:
        switch (inst.rs) {
        case 16:
            return tlb_table[inst.func];
        }

        return cop0_table[inst.rs];
    case 17:
        switch (inst.rs) {
        case 16:
            return fpu_s_table[inst.func];
        }

        return cop1_table[inst.rs];
    case 18:
        return cop2_table[inst.rs];
    case 28:
        switch (inst.func) {
        case 9:
            return mmi2_table[inst.imm5];
        case 40:
            return mmi1_table[inst.imm5];
        case 41:
            return mmi3_table[inst.imm5];
        }

        return mmi_table[inst.func];
    }

    return primary_table[inst.opcode];
}

void InterpreterTable::Execute(EECore& cpu, CPUInstruction inst) {
    InterpreterInstruction handler = GetInterpreterInstruction(cpu, inst);
    handler(cpu, inst);
}

void InterpreterTable::RegisterOpcode(InterpreterInstruction handler, int index, InstructionTable table) {
    switch (table) {
    case InstructionTable::Primary:
        primary_table[index] = handler;
        break;
    case InstructionTable::Secondary:
        secondary_table[index] = handler;
        break;
    case InstructionTable::RegImm:
        regimm_table[index] = handler;
        break;
    case InstructionTable::COP0:
        cop0_table[index] = handler;
        break;
    case InstructionTable::COP1:
        cop1_table[index] = handler;
        break;
    case InstructionTable::FPUS:
        fpu_s_table[index] = handler;
        break;
    case InstructionTable::COP2:
        cop2_table[index] = handler;
        break;
    case InstructionTable::TLB:
        tlb_table[index] = handler;
        break;
    case InstructionTable::MMI:
        mmi_table[index] = handler;
        break;
    case InstructionTable::MMI1:
        mmi1_table[index] = handler;
        break;
    case InstructionTable::MMI2:
        mmi2_table[index] = handler;
        break;
    case InstructionTable::MMI3:
        mmi3_table[index] = handler;
        break;
    }
}