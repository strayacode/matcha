#include "common/log_file.h"
#include "core/iop/interpreter/interpreter.h"
#include "core/iop/disassembler.h"
#include "core/system.h"

IOPInterpreter::IOPInterpreter(System* system) : IOPCore(system) {
    for (int i = 0; i < 64; i++) {
        primary_table[i] = &IOPInterpreter::UndefinedInstruction;
        secondary_table[i] = &IOPInterpreter::UndefinedInstruction;
    }

    // primary instructions
    RegisterOpcode(&IOPInterpreter::SecondaryInstruction, 0, InstructionTable::Primary);
    RegisterOpcode(&IOPInterpreter::bcondz, 1, InstructionTable::Primary);
    RegisterOpcode(&IOPInterpreter::j, 2, InstructionTable::Primary);
    RegisterOpcode(&IOPInterpreter::jal, 3, InstructionTable::Primary);
    RegisterOpcode(&IOPInterpreter::beq, 4, InstructionTable::Primary);
    RegisterOpcode(&IOPInterpreter::bne, 5, InstructionTable::Primary);
    RegisterOpcode(&IOPInterpreter::blez, 6, InstructionTable::Primary);
    RegisterOpcode(&IOPInterpreter::bgtz, 7, InstructionTable::Primary);
    RegisterOpcode(&IOPInterpreter::addi, 8, InstructionTable::Primary);
    RegisterOpcode(&IOPInterpreter::addiu, 9, InstructionTable::Primary);
    RegisterOpcode(&IOPInterpreter::slti, 10, InstructionTable::Primary);   
    RegisterOpcode(&IOPInterpreter::sltiu, 11, InstructionTable::Primary); 
    RegisterOpcode(&IOPInterpreter::andi, 12, InstructionTable::Primary);
    RegisterOpcode(&IOPInterpreter::ori, 13, InstructionTable::Primary);
    RegisterOpcode(&IOPInterpreter::lui, 15, InstructionTable::Primary);
    RegisterOpcode(&IOPInterpreter::COP0Instruction, 16, InstructionTable::Primary);
    RegisterOpcode(&IOPInterpreter::lb, 32, InstructionTable::Primary);
    RegisterOpcode(&IOPInterpreter::lh, 33, InstructionTable::Primary);
    RegisterOpcode(&IOPInterpreter::lw, 35, InstructionTable::Primary);
    RegisterOpcode(&IOPInterpreter::lbu, 36, InstructionTable::Primary);
    RegisterOpcode(&IOPInterpreter::lhu, 37, InstructionTable::Primary);
    RegisterOpcode(&IOPInterpreter::sb, 40, InstructionTable::Primary);
    RegisterOpcode(&IOPInterpreter::sh, 41, InstructionTable::Primary);
    RegisterOpcode(&IOPInterpreter::sw, 43, InstructionTable::Primary);

    // secondary instructions
    RegisterOpcode(&IOPInterpreter::sll, 0, InstructionTable::Secondary);
    RegisterOpcode(&IOPInterpreter::srl, 2, InstructionTable::Secondary);
    RegisterOpcode(&IOPInterpreter::sra, 3, InstructionTable::Secondary);
    RegisterOpcode(&IOPInterpreter::sllv, 4, InstructionTable::Secondary);
    RegisterOpcode(&IOPInterpreter::srlv, 6, InstructionTable::Secondary);
    RegisterOpcode(&IOPInterpreter::jr, 8, InstructionTable::Secondary);
    RegisterOpcode(&IOPInterpreter::jalr, 9, InstructionTable::Secondary);
    RegisterOpcode(&IOPInterpreter::syscall_exception, 12, InstructionTable::Secondary);
    RegisterOpcode(&IOPInterpreter::mfhi, 16, InstructionTable::Secondary);
    RegisterOpcode(&IOPInterpreter::mthi, 17, InstructionTable::Secondary);
    RegisterOpcode(&IOPInterpreter::mflo, 18, InstructionTable::Secondary);
    RegisterOpcode(&IOPInterpreter::mtlo, 19, InstructionTable::Secondary);
    RegisterOpcode(&IOPInterpreter::mult, 24, InstructionTable::Secondary);
    RegisterOpcode(&IOPInterpreter::multu, 25, InstructionTable::Secondary);
    RegisterOpcode(&IOPInterpreter::div, 26, InstructionTable::Secondary);
    RegisterOpcode(&IOPInterpreter::divu, 27, InstructionTable::Secondary);
    RegisterOpcode(&IOPInterpreter::add, 32, InstructionTable::Secondary);
    RegisterOpcode(&IOPInterpreter::addu, 33, InstructionTable::Secondary);
    RegisterOpcode(&IOPInterpreter::subu, 35, InstructionTable::Secondary);
    RegisterOpcode(&IOPInterpreter::andd, 36, InstructionTable::Secondary);
    RegisterOpcode(&IOPInterpreter::orr, 37, InstructionTable::Secondary);
    RegisterOpcode(&IOPInterpreter::xorr, 38, InstructionTable::Secondary);
    RegisterOpcode(&IOPInterpreter::nor, 39, InstructionTable::Secondary);
    RegisterOpcode(&IOPInterpreter::slt, 42, InstructionTable::Secondary);
    RegisterOpcode(&IOPInterpreter::sltu, 43, InstructionTable::Secondary);
}

void IOPInterpreter::Reset() {
    for (int i = 0; i < 32; i++) {
        regs.gpr[i] = 0;
    }

    regs.pc = 0xBFC00000;
    regs.next_pc = 0;
    regs.hi = 0;
    regs.lo = 0;
    inst.data = 0;
    branch_delay = false;
    branch = false;

    cop0.Reset();
    interrupt_controller.Reset();
}

void IOPInterpreter::Run(int cycles) {
    while (cycles--) {
        inst = CPUInstruction{ReadWord(regs.pc)};

        if (regs.pc == 0x00012C48 || regs.pc == 0x0001420C || regs.pc == 0x0001430C) {
            IOPPuts();
        }

        (this->*primary_table[inst.opcode])();

        regs.pc += 4;

        if (branch_delay) {
            if (branch) {
                regs.pc = regs.next_pc;
                branch_delay = false;
                branch = false;
            } else {
                branch = true;
            }
        }

        CheckInterrupts();
    }
}

void IOPInterpreter::RegisterOpcode(InstructionHandler handler, int index, InstructionTable table) {
    if (table == InstructionTable::Primary) {
        primary_table[index] = handler;
    } else {
        secondary_table[index] = handler;
    }
}

void IOPInterpreter::UndefinedInstruction() {
    log_fatal("%s %08x at %08x (primary = %d, secondary = %d, regimm = %d) is undefined", IOPDisassembleInstruction(inst, regs.pc).c_str(), inst.data, regs.pc, inst.opcode, inst.func, inst.rt);
}

void IOPInterpreter::SecondaryInstruction() {
    (this->*secondary_table[inst.func])();
}

void IOPInterpreter::COP0Instruction() {
    u8 format = inst.rs;

    switch (format) {
    case 0:
        mfc0();
        break;
    case 4:
        mtc0();
        break;
    case 16:
        rfe();
        break;
    default:
        log_fatal("handle %d", format);
    }
}

void IOPInterpreter::IOPPuts() {
    u32 address = GetReg(5);
    u32 length = GetReg(6);
    
    for (int i = 0; i < length; i++) {
        LogFile::Get().Log("%c", system->memory.iop_ram[address & 0x1FFFFF]);
        address++;
    }
}