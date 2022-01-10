#include <core/ee/interpreter/interpreter.h>
#include <core/system.h>

EEInterpreter::EEInterpreter(System* system) : EECore(system) {
    for (int i = 0; i < 64; i++) {
        primary_table[i] = &EEInterpreter::UndefinedInstruction;
        secondary_table[i] = &EEInterpreter::UndefinedInstruction;
    }

    // primary instructions
    RegisterOpcode(&EEInterpreter::SecondaryInstruction, 0, InstructionTable::Primary);
    RegisterOpcode(&EEInterpreter::RegImmInstruction, 1, InstructionTable::Primary);
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
    RegisterOpcode(&EEInterpreter::COP0Instruction, 16, InstructionTable::Primary);
    RegisterOpcode(&EEInterpreter::COP1Instruction, 17, InstructionTable::Primary);
    RegisterOpcode(&EEInterpreter::COP2Instruction, 18, InstructionTable::Primary);
    RegisterOpcode(&EEInterpreter::beql, 20, InstructionTable::Primary);
    RegisterOpcode(&EEInterpreter::bnel, 21, InstructionTable::Primary);
    RegisterOpcode(&EEInterpreter::daddiu, 25, InstructionTable::Primary);
    RegisterOpcode(&EEInterpreter::ldl, 26, InstructionTable::Primary);
    RegisterOpcode(&EEInterpreter::ldr, 27, InstructionTable::Primary);
    RegisterOpcode(&EEInterpreter::MMIInstruction, 28, InstructionTable::Primary);
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
    RegisterOpcode(&EEInterpreter::syscall, 12, InstructionTable::Secondary);
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
    RegisterOpcode(&EEInterpreter::dsll, 56, InstructionTable::Secondary);
    RegisterOpcode(&EEInterpreter::dsrl, 58, InstructionTable::Secondary);
    RegisterOpcode(&EEInterpreter::dsll32, 60, InstructionTable::Secondary);
    RegisterOpcode(&EEInterpreter::dsrl32, 62, InstructionTable::Secondary);
    RegisterOpcode(&EEInterpreter::dsra32, 63, InstructionTable::Secondary);

    log_file.SetPath("../../log-stuff/otterstation.log");
}

void EEInterpreter::Reset() {
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

void EEInterpreter::Run(int cycles) {
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

        system->ee_cop0.CountUp();
    }
}

u8 EEInterpreter::ReadByte(u32 addr) {
    return system->memory.EERead<u8>(addr);
}

u16 EEInterpreter::ReadHalf(u32 addr) {
    return system->memory.EERead<u16>(addr);
}

u32 EEInterpreter::ReadWord(u32 addr) {
    return system->memory.EERead<u32>(addr);
}

u64 EEInterpreter::ReadDouble(u32 addr) {
    return system->memory.EERead<u64>(addr);
}

void EEInterpreter::WriteByte(u32 addr, u8 data) {
    system->memory.EEWrite<u8>(addr, data);
}

void EEInterpreter::WriteHalf(u32 addr, u16 data) {
    system->memory.EEWrite<u16>(addr, data);
}

void EEInterpreter::WriteWord(u32 addr, u32 data) {
    system->memory.EEWrite<u32>(addr, data);
}

void EEInterpreter::WriteDouble(u32 addr, u64 data) {
    system->memory.EEWrite<u64>(addr, data);
}

void EEInterpreter::WriteQuad(u32 addr, u128 data) {
    system->memory.EEWrite<u128>(addr, data);
}

void EEInterpreter::RegisterOpcode(InstructionHandler handler, int index, InstructionTable table) {
    if (table == InstructionTable::Primary) {
        primary_table[index] = handler;
    } else {
        secondary_table[index] = handler;
    }
}

void EEInterpreter::COP0Instruction() {
    u8 format = inst.i.rs;

    switch (format) {
    case 0:
        mfc0();
        break;
    case 4:
        mtc0();
        break;
    case 16:
        TLBInstruction();
        break;
    default:
        log_fatal("handle %d", format);
    }
}

void EEInterpreter::UndefinedInstruction() {
    log_fatal("%s = %08x at %08x (primary = %d, secondary = %d, regimm = %d) is undefined", EEDisassembleInstruction(inst, regs.pc).c_str(), inst.data, regs.pc, inst.i.opcode, inst.r.func, inst.i.rt);
}

void EEInterpreter::SecondaryInstruction() {
    (this->*secondary_table[inst.r.func])();
}

void EEInterpreter::TLBInstruction() {
    switch (inst.r.func) {
    case 2:
        tlbwi();
        break;
    case 24:
        eret();
        break;
    case 56:
        ei();
        break;
    case 57:
        di();
        break;
    default:
        log_fatal("undefined %d", inst.r.func);
        UndefinedInstruction();
    }
}

void EEInterpreter::RegImmInstruction() {
    switch (inst.i.rt) {
    case 0:
        bltz();
        break;
    case 1:
        bgez();
        break;
    case 2:
        bltzl();
        break;
    default:
        UndefinedInstruction();
    }
}

void EEInterpreter::MMIInstruction() {
    switch (inst.r.func) {
    case 9:
        MMI2Instruction();
        break;
    case 18:
        mflo1();
        break;
    case 24:
        mult1();
        break;
    case 27:
        divu1();
        break;
    case 40:
        MMI1Instruction();
        break;
    case 41:
        MMI3Instruction();
        break;
    default:
        log_fatal("handle %d", inst.r.func);
        UndefinedInstruction();
    }
}

void EEInterpreter::MMI1Instruction() {
    switch (inst.r.sa) {
    case 16:
        padduw();
        break;
    default:
        log_fatal("handle %d", inst.r.sa);
        UndefinedInstruction();
    }
}

void EEInterpreter::MMI2Instruction() {
    switch (inst.r.sa) {
    default:
        log_fatal("handle %d", inst.r.sa);
        UndefinedInstruction();
    }
}

void EEInterpreter::MMI3Instruction() {
    switch (inst.r.sa) {
    case 18:
        por();
        break;
    default:
        log_fatal("handle %d", inst.r.sa);
        UndefinedInstruction();
    }
}

void EEInterpreter::COP2Instruction() {
    // TODO: handle vu0 instructions
    // switch (inst.i.rs) {
    // case 2:
    //     cfc2();
    //     break;
    // case 6:
    //     ctc2();
    //     break;
    // default:
    //     log_fatal("handle %d", inst.i.rs);
    //     UndefinedInstruction();
    // }
}

void EEInterpreter::COP1Instruction() {
    // TODO: handle fpu instructions
}

void EEInterpreter::RecordRegisters() {
    for (int i = 0; i < 32; i++) {
        log_file.Log("%016lx ", GetReg<u64>(i));
    }

    log_file.Log("pc: %08x hi: %016lx lo: %016lx hi1: %016lx lo1: %016lx sa: %016lx %08x\n", regs.pc, regs.hi, regs.lo, regs.hi1, regs.lo1, regs.sa, inst.data);
}

void EEInterpreter::DoException(u32 target, ExceptionType exception) {
    u32 status = system->ee_cop0.GetReg(12);
    u32 cause = system->ee_cop0.GetReg(13);

    bool level2_exception = static_cast<int>(exception) >= 14;
    int code = level2_exception ? static_cast<int>(exception) - 14 : static_cast<int>(exception);

    if (level2_exception) {
        log_fatal("handle level 2 exception");
    } else {
        cause |= (code << 2);
        if (branch_delay) {
            system->ee_cop0.SetReg(14, regs.pc - 4);
            cause |= (1 << 31);
        } else {
            system->ee_cop0.SetReg(14, regs.pc);
            cause &= ~(1 << 31);
        }

        status |= (1 << 1);
        regs.pc = target - 4;
    }

    system->ee_cop0.SetReg(12, status);
    system->ee_cop0.SetReg(13, cause);
}