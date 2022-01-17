#include <core/iop/disassembler.h>

static std::array<std::string, 32> reg_names = {
    "zero", "at", "v0", "v1", "a0", "a1", "a2", "a3",
    "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
    "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
    "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra",
};

enum class InstructionType {
    Immediate,
    Jump,
    Register,
    None,
};

struct DisassemblyInfo {
    std::string format;
    InstructionType type;
};

static std::array<DisassemblyInfo, 64> primary_table = {
    DisassemblyInfo{"secondary", InstructionType::None},
    DisassemblyInfo{"bcondz", InstructionType::None},
    DisassemblyInfo{"j $target", InstructionType::Jump},
    DisassemblyInfo{"jal $target", InstructionType::Jump},
    DisassemblyInfo{"beq $rs, $rt, $offset", InstructionType::Immediate},
    DisassemblyInfo{"bne $rs, $rt, $offset", InstructionType::Immediate},
    DisassemblyInfo{"blez $rs, $offset", InstructionType::Immediate},
    DisassemblyInfo{"bgtz $rs, $offset", InstructionType::Immediate},
    DisassemblyInfo{"addi $rt, $rs, $imm", InstructionType::Immediate},
    DisassemblyInfo{"addiu $rt, $rs, $imm", InstructionType::Immediate},
    DisassemblyInfo{"slti $rt, $rs, $imm", InstructionType::Immediate},
    DisassemblyInfo{"sltiu $rt, $rs, $imm", InstructionType::Immediate},
    DisassemblyInfo{"andi $rt, $rs, $imm", InstructionType::Immediate},
    DisassemblyInfo{"ori $rt, $rs, $imm", InstructionType::Immediate},
    DisassemblyInfo{"xori $rt, $rs, $imm", InstructionType::Immediate},
    DisassemblyInfo{"lui $rt, $imm", InstructionType::Immediate},
    DisassemblyInfo{"cop0", InstructionType::None},
    DisassemblyInfo{"cop1", InstructionType::None},
    DisassemblyInfo{"cop2", InstructionType::None},
    DisassemblyInfo{"cop3", InstructionType::None},
    DisassemblyInfo{"illegal", InstructionType::None},
    DisassemblyInfo{"illegal", InstructionType::None},
    DisassemblyInfo{"illegal", InstructionType::None},
    DisassemblyInfo{"illegal", InstructionType::None},
    DisassemblyInfo{"illegal", InstructionType::None},
    DisassemblyInfo{"illegal", InstructionType::None},
    DisassemblyInfo{"illegal", InstructionType::None},
    DisassemblyInfo{"illegal", InstructionType::None},
    DisassemblyInfo{"illegal", InstructionType::None},
    DisassemblyInfo{"illegal", InstructionType::None},
    DisassemblyInfo{"illegal", InstructionType::None},
    DisassemblyInfo{"illegal", InstructionType::None},
    DisassemblyInfo{"lb $rt, $imm($rs)", InstructionType::Immediate},
    DisassemblyInfo{"lh $rt, $imm($rs)", InstructionType::Immediate},
    DisassemblyInfo{"lwl $rt, $imm($rs)", InstructionType::Immediate},
    DisassemblyInfo{"lw $rt, $imm($rs)", InstructionType::Immediate},
    DisassemblyInfo{"lbu $rt, $imm($rs)", InstructionType::Immediate},
    DisassemblyInfo{"lhu $rt, $imm($rs)", InstructionType::Immediate},
    DisassemblyInfo{"lwr $rt, $imm($rs)", InstructionType::Immediate},
    DisassemblyInfo{"illegal", InstructionType::None},
    DisassemblyInfo{"sb $rt, $imm($rs)", InstructionType::Immediate},
    DisassemblyInfo{"sh $rt, $imm($rs)", InstructionType::Immediate},
    DisassemblyInfo{"swl $rt, $imm($rs)", InstructionType::Immediate},
    DisassemblyInfo{"sw $rt, $imm($rs)", InstructionType::Immediate},
    DisassemblyInfo{"illegal", InstructionType::None},
    DisassemblyInfo{"illegal", InstructionType::None},
    DisassemblyInfo{"swr $rt, $imm($rs)", InstructionType::Immediate},
    DisassemblyInfo{"illegal", InstructionType::None},
    DisassemblyInfo{"lwc0", InstructionType::None},
    DisassemblyInfo{"lwc1", InstructionType::None},
    DisassemblyInfo{"lwc2", InstructionType::None},
    DisassemblyInfo{"lwc3", InstructionType::None},
    DisassemblyInfo{"illegal", InstructionType::None},
    DisassemblyInfo{"illegal", InstructionType::None},
    DisassemblyInfo{"illegal", InstructionType::None},
    DisassemblyInfo{"illegal", InstructionType::None},
    DisassemblyInfo{"swc0", InstructionType::None},
    DisassemblyInfo{"swc1", InstructionType::None},
    DisassemblyInfo{"swc2", InstructionType::None},
    DisassemblyInfo{"swc3", InstructionType::None},
    DisassemblyInfo{"illegal", InstructionType::None},
    DisassemblyInfo{"illegal", InstructionType::None},
    DisassemblyInfo{"illegal", InstructionType::None},
    DisassemblyInfo{"illegal", InstructionType::None},
};

static std::array<DisassemblyInfo, 64> secondary_table = {
    DisassemblyInfo{"sll $rd, $rt, $sa", InstructionType::Register},
    DisassemblyInfo{"illegal", InstructionType::None},
    DisassemblyInfo{"srl $rd, $rt, $sa", InstructionType::Register},
    DisassemblyInfo{"sra $rd, $rt, $sa", InstructionType::Register},
    DisassemblyInfo{"sllv $rd, $rt, $rs", InstructionType::Register},
    DisassemblyInfo{"illegal", InstructionType::None},
    DisassemblyInfo{"srlv $rd, $rt, $rs", InstructionType::Register},
    DisassemblyInfo{"srav $rd, $rt, $rs", InstructionType::Register},
    DisassemblyInfo{"jr $rs", InstructionType::Immediate},
    DisassemblyInfo{"jalr $rd, $rs", InstructionType::Register},
    DisassemblyInfo{"movz $rd, $rs, $rt", InstructionType::Register},
    DisassemblyInfo{"movn $rd, $rs, $rt", InstructionType::Register},
    DisassemblyInfo{"syscall", InstructionType::None},
    DisassemblyInfo{"break", InstructionType::None},
    DisassemblyInfo{"illegal", InstructionType::None},
    DisassemblyInfo{"sync $sa", InstructionType::Register},
    DisassemblyInfo{"mfhi $rd", InstructionType::Register},
    DisassemblyInfo{"mthi $rs", InstructionType::Register},
    DisassemblyInfo{"mflo $rd", InstructionType::Register},
    DisassemblyInfo{"mtlo $rs", InstructionType::Register},
    DisassemblyInfo{"dsllv $rd, $rt, $rs", InstructionType::Register},
    DisassemblyInfo{"illegal", InstructionType::None},
    DisassemblyInfo{"dsrlv $rd, $rt, $rs", InstructionType::Register},
    DisassemblyInfo{"dsrav $rd, $rt, $rs", InstructionType::Register},
    DisassemblyInfo{"mult $rd, $rs, $rt", InstructionType::Register},
    DisassemblyInfo{"multu $rd, $rs, $rt", InstructionType::Register},
    DisassemblyInfo{"div $rs, $rt", InstructionType::Register},
    DisassemblyInfo{"divu $rs, $rt", InstructionType::Register},
    DisassemblyInfo{"illegal", InstructionType::None},
    DisassemblyInfo{"illegal", InstructionType::None},
    DisassemblyInfo{"illegal", InstructionType::None},
    DisassemblyInfo{"illegal", InstructionType::None},
    DisassemblyInfo{"add $rd, $rs, $rt", InstructionType::Register},
    DisassemblyInfo{"addu $rd, $rs, $rt", InstructionType::Register},
    DisassemblyInfo{"sub $rd, $rs, $rt", InstructionType::Register},
    DisassemblyInfo{"subu $rd, $rs, $rt", InstructionType::Register},
    DisassemblyInfo{"and $rd, $rs, $rt", InstructionType::Register},
    DisassemblyInfo{"or $rd, $rs, $rt", InstructionType::Register},
    DisassemblyInfo{"xor $rd, $rs, $rt", InstructionType::Register},
    DisassemblyInfo{"nor $rd, $rs, $rt", InstructionType::Register},
    DisassemblyInfo{"mfsa $rd", InstructionType::Register},
    DisassemblyInfo{"mtsa $rs", InstructionType::Register},
    DisassemblyInfo{"slt $rd, $rs, $rt", InstructionType::Register},
    DisassemblyInfo{"sltu $rd, $rs, $rt", InstructionType::Register},
    DisassemblyInfo{"dadd $rd, $rs, $rt", InstructionType::Register},
    DisassemblyInfo{"daddu $rd, $rs, $rt", InstructionType::Register},
    DisassemblyInfo{"dsub $rd, $rs, $rt", InstructionType::Register},
    DisassemblyInfo{"dsubu $rd, $rs, $rt", InstructionType::Register},
    DisassemblyInfo{"tge $rs, $rt", InstructionType::Register},
    DisassemblyInfo{"tgeu $rs, $rt", InstructionType::Register},
    DisassemblyInfo{"tlt $rs, $rt", InstructionType::Register},
    DisassemblyInfo{"tltu $rs, $rt", InstructionType::Register},
    DisassemblyInfo{"teq $rs, $rt", InstructionType::Register},
    DisassemblyInfo{"illegal", InstructionType::None},
    DisassemblyInfo{"tne $rs, $rt", InstructionType::Register},
    DisassemblyInfo{"illegal", InstructionType::None},
    DisassemblyInfo{"dsll $rd, $rt, $sa", InstructionType::Register},
    DisassemblyInfo{"illegal", InstructionType::None},
    DisassemblyInfo{"dsrl $rd, $rt, $sa", InstructionType::Register},
    DisassemblyInfo{"dsra $rd, $rt, $sa", InstructionType::Register},
    DisassemblyInfo{"dsll32 $rd, $rt, $sa", InstructionType::Register},
    DisassemblyInfo{"illegal", InstructionType::None},
    DisassemblyInfo{"dsrl32 $rd, $rt, $sa", InstructionType::Register},
    DisassemblyInfo{"dsra32 $rd, $rt, $sa", InstructionType::Register},
};

static std::map<int, DisassemblyInfo> cop0_table = {
    {0, DisassemblyInfo{"mfc0 $rt, $cd", InstructionType::Register}},
    {4, DisassemblyInfo{"mtc0 $rt, $cd", InstructionType::Register}},
};

static std::map<int, std::string> cop0_names = {
    {0, "Index"},
    {1, "Random"},
    {2, "EntryLo0"},
    {3, "EntryLo1"},
    {4, "Context"},
    {5, "PageMask"},
    {6, "Wired"},
    {8, "BadVAddr"},
    {9, "Count"},
    {10, "EntryHi"},
    {11, "Compare"},
    {12, "Status"},
    {13, "Cause"},
    {14, "EPC"},
    {15, "PRId"},
    {16, "Config"},
    {23, "BadPAddr"},
    {24, "Debug"},
    {25, "Perf"},
    {28, "TagLo"},
    {29, "TagHi"},
    {30, "ErrorEPC"},
};

template <typename T>
static std::string ConvertHex(T data) {
    std::stringstream stream;

    stream << "0x" << std::hex << data;

    return stream.str();
}

static std::string DisassembleImmediate(CPUInstruction inst, u32 pc, std::string format) {
    std::string disassembled;
    u64 i = 0;

    while (i < format.length()) {
        if (format.compare(i, 3, "$rt") == 0) {
            disassembled += "$" + reg_names[inst.rt];
            i += 3;
        } else if (format.compare(i, 3, "$rs") == 0) {
            disassembled += "$" + reg_names[inst.rs];
            i += 3;
        } else if (format.compare(i, 4, "$imm") == 0) {
            disassembled += ConvertHex<u16>(inst.imm);
            i += 4;
        } else if (format.compare(i, 7, "$offset") == 0) {
            disassembled += ConvertHex<u32>(pc + (((s16)inst.imm) << 2) + 4);
            i += 7;
        } else {
            disassembled += format[i];
            i++;
        }
    }

    return disassembled;
}

static std::string DisassembleJump(CPUInstruction inst, u32 pc, std::string format) {
    std::string disassembled;
    u64 i = 0;

    while (i < format.length()) {
        if (format.compare(i, 7, "$target") == 0) {
            disassembled += ConvertHex<u32>(((pc + 4) & 0xF0000000) + (inst.offset << 2));
            i += 7;
        } else {
            disassembled += format[i];
            i++;
        }
    }

    return disassembled;
}

static std::string DisassembleRegister(CPUInstruction inst, u32 pc, std::string format) {
    std::string disassembled;
    u64 i = 0;

    while (i < format.length()) {
        if (format.compare(i, 3, "$rt") == 0) {
            disassembled += "$" + reg_names[inst.rt];
            i += 3;
        } else if (format.compare(i, 3, "$rd") == 0) {
            disassembled += "$" + reg_names[inst.rd];
            i += 3;
        } else if (format.compare(i, 3, "$cd") == 0) {
            disassembled += cop0_names[inst.rd];
            i += 3;
        } else if (format.compare(i, 3, "$rs") == 0) {
            disassembled += "$" + reg_names[inst.rs];
            i += 3;
        } else if (format.compare(i, 3, "$sa") == 0) {
            disassembled += ConvertHex<u16>(inst.imm5);
            i += 3;
        } else {
            disassembled += format[i];
            i++;
        }
    }

    return disassembled;
}

std::string IOPDisassembleInstruction(CPUInstruction inst, u32 pc) {
    std::string disassembled;

    DisassemblyInfo info = primary_table[inst.opcode];

    if (info.format.compare("secondary") == 0) {
        info = secondary_table[inst.func];
    } else if (info.format.compare("cop0") == 0) {
        info = cop0_table[inst.rs];
    }

    switch (info.type) {
    case InstructionType::Immediate:
        disassembled = DisassembleImmediate(inst, pc, info.format);
        break;
    case InstructionType::Jump:
        disassembled = DisassembleJump(inst, pc, info.format);
        break;
    case InstructionType::Register:
        disassembled = DisassembleRegister(inst, pc, info.format);
        break;
    case InstructionType::None:
        disassembled = info.format;
        break;
    }

    return disassembled;
}

std::string IOPGetRegisterName(int reg) {
    return reg_names[reg];
}

std::string IOPCOP0GetRegisterName(int reg) {
    return cop0_names[reg];
}