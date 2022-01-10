#include <core/iop/interpreter/interpreter.h>
#include <core/iop/disassembler.h>
#include <core/system.h>

IOPInterpreter::IOPInterpreter(System* system) : IOPCore(system) {
    for (int i = 0; i < 64; i++) {
        primary_table[i] = &IOPInterpreter::UndefinedInstruction;
        secondary_table[i] = &IOPInterpreter::UndefinedInstruction;
    }

    // primary instructions
    RegisterOpcode(&IOPInterpreter::SecondaryInstruction, 0, InstructionTable::Primary);
    RegisterOpcode(&IOPInterpreter::j, 2, InstructionTable::Primary);
    RegisterOpcode(&IOPInterpreter::jal, 3, InstructionTable::Primary);
    RegisterOpcode(&IOPInterpreter::beq, 4, InstructionTable::Primary);
    RegisterOpcode(&IOPInterpreter::bne, 5, InstructionTable::Primary);
    RegisterOpcode(&IOPInterpreter::blez, 6, InstructionTable::Primary);
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
    RegisterOpcode(&IOPInterpreter::sw, 43, InstructionTable::Primary);

    // secondary instructions
    RegisterOpcode(&IOPInterpreter::sll, 0, InstructionTable::Secondary);
    RegisterOpcode(&IOPInterpreter::srl, 2, InstructionTable::Secondary);
    RegisterOpcode(&IOPInterpreter::sra, 3, InstructionTable::Secondary);
    RegisterOpcode(&IOPInterpreter::jr, 8, InstructionTable::Secondary);
    RegisterOpcode(&IOPInterpreter::addu, 33, InstructionTable::Secondary);
    RegisterOpcode(&IOPInterpreter::subu, 35, InstructionTable::Secondary);
    RegisterOpcode(&IOPInterpreter::andd, 36, InstructionTable::Secondary);
    RegisterOpcode(&IOPInterpreter::orr, 37, InstructionTable::Secondary);
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
}

void IOPInterpreter::Run(int cycles) {
    while (cycles--) {
        inst = CPUInstruction{ReadWord(regs.pc)};

        (this->*primary_table[inst.i.opcode])();

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
    }
}

u8 IOPInterpreter::ReadByte(u32 addr) {
    return system->memory.IOPRead<u8>(addr);
}

u16 IOPInterpreter::ReadHalf(u32 addr) {
    return system->memory.IOPRead<u16>(addr);
}

u32 IOPInterpreter::ReadWord(u32 addr) {
    return system->memory.IOPRead<u32>(addr);
}

void IOPInterpreter::WriteByte(u32 addr, u8 data) {
    system->memory.IOPWrite<u8>(addr, data);
}

void IOPInterpreter::WriteHalf(u32 addr, u16 data) {
    system->memory.IOPWrite<u16>(addr, data);
}

void IOPInterpreter::WriteWord(u32 addr, u32 data) {
    system->memory.IOPWrite<u32>(addr, data);
}

void IOPInterpreter::RegisterOpcode(InstructionHandler handler, int index, InstructionTable table) {
    if (table == InstructionTable::Primary) {
        primary_table[index] = handler;
    } else {
        secondary_table[index] = handler;
    }
}

void IOPInterpreter::UndefinedInstruction() {
    log_fatal("%s %08x at %08x (primary = %d, secondary = %d, regimm = %d) is undefined", IOPDisassembleInstruction(inst, regs.pc).c_str(), inst.data, regs.pc, inst.i.opcode, inst.r.func, inst.i.rt);
}

void IOPInterpreter::SecondaryInstruction() {
    (this->*secondary_table[inst.r.func])();
}

void IOPInterpreter::COP0Instruction() {
    u8 format = inst.i.rs;

    switch (format) {
    case 0:
        mfc0();
        break;
    case 4:
        mtc0();
        break;
    default:
        log_fatal("handle %d", format);
    }
}