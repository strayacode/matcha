#include <array>
#include <sstream>
#include <map>
#include "common/log.h"
#include "core/ee/disassembler.h"

namespace ee {

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
    DisassemblyInfo{"regimm", InstructionType::None},
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
    DisassemblyInfo{"illegal", InstructionType::None},
    DisassemblyInfo{"beql $rs, $rt, $offset", InstructionType::Immediate},
    DisassemblyInfo{"bnel $rs, $rt, $offset", InstructionType::Immediate},
    DisassemblyInfo{"blezl $rs, $offset", InstructionType::Immediate},
    DisassemblyInfo{"bgtzl $rs, $offset", InstructionType::Immediate},
    DisassemblyInfo{"daddi $rt, $rs, $imm", InstructionType::Immediate},
    DisassemblyInfo{"daddiu $rt, $rs, $imm", InstructionType::Immediate},
    DisassemblyInfo{"ldl $rt, $imm($rs)", InstructionType::Immediate},
    DisassemblyInfo{"ldr $rt, $imm($rs)", InstructionType::Immediate},
    DisassemblyInfo{"mmi", InstructionType::None},
    DisassemblyInfo{"illegal", InstructionType::None},
    DisassemblyInfo{"lq $rt, $imm($rs)", InstructionType::Immediate},
    DisassemblyInfo{"sq $rt, $imm($rs)", InstructionType::Immediate},
    DisassemblyInfo{"lb $rt, $imm($rs)", InstructionType::Immediate},
    DisassemblyInfo{"lh $rt, $imm($rs)", InstructionType::Immediate},
    DisassemblyInfo{"lwl $rt, $imm($rs)", InstructionType::Immediate},
    DisassemblyInfo{"lw $rt, $imm($rs)", InstructionType::Immediate},
    DisassemblyInfo{"lbu $rt, $imm($rs)", InstructionType::Immediate},
    DisassemblyInfo{"lhu $rt, $imm($rs)", InstructionType::Immediate},
    DisassemblyInfo{"lwr $rt, $imm($rs)", InstructionType::Immediate},
    DisassemblyInfo{"lwu $rt, $imm($rs)", InstructionType::Immediate},
    DisassemblyInfo{"sb $rt, $imm($rs)", InstructionType::Immediate},
    DisassemblyInfo{"sh $rt, $imm($rs)", InstructionType::Immediate},
    DisassemblyInfo{"swl $rt, $imm($rs)", InstructionType::Immediate},
    DisassemblyInfo{"sw $rt, $imm($rs)", InstructionType::Immediate},
    DisassemblyInfo{"sdl $rt, $imm($rs)", InstructionType::Immediate},
    DisassemblyInfo{"sdr $rt, $imm($rs)", InstructionType::Immediate},
    DisassemblyInfo{"swr $rt, $imm($rs)", InstructionType::Immediate},
    DisassemblyInfo{"cache", InstructionType::None},
    DisassemblyInfo{"illegal", InstructionType::None},
    DisassemblyInfo{"lwc1 $ft, $imm($rs)", InstructionType::Immediate},
    DisassemblyInfo{"illegal", InstructionType::None},
    DisassemblyInfo{"pref $rt, $imm($rs)", InstructionType::Immediate},
    DisassemblyInfo{"illegal", InstructionType::None},
    DisassemblyInfo{"illegal", InstructionType::None},
    DisassemblyInfo{"lqc2 vu0 thing", InstructionType::None},
    DisassemblyInfo{"ld $rt, $imm($rs)", InstructionType::Immediate},
    DisassemblyInfo{"illegal", InstructionType::None},
    DisassemblyInfo{"swc1 $ft, $imm($rs)", InstructionType::Immediate},
    DisassemblyInfo{"illegal", InstructionType::None},
    DisassemblyInfo{"illegal", InstructionType::None},
    DisassemblyInfo{"illegal", InstructionType::None},
    DisassemblyInfo{"illegal", InstructionType::None},
    DisassemblyInfo{"sqc2 vu0 thing", InstructionType::None},
    DisassemblyInfo{"sd $rt, $imm($rs)", InstructionType::Immediate},
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

static std::map<int, DisassemblyInfo> regimm_table = {
    {0, DisassemblyInfo{"bltz $rs, $offset", InstructionType::Immediate}},
    {1, DisassemblyInfo{"bgez $rs, $offset", InstructionType::Immediate}},
    {2, DisassemblyInfo{"bltzl $rs, $offset", InstructionType::Immediate}},
};

static std::map<int, DisassemblyInfo> cop0_table = {
    {0, DisassemblyInfo{"mfc0 $rt, $cd", InstructionType::Register}},
    {4, DisassemblyInfo{"mtc0 $rt, $cd", InstructionType::Register}},
    {16, DisassemblyInfo{"tlb", InstructionType::None}},
};

static std::map<int, DisassemblyInfo> cop1_table = {
    {0, DisassemblyInfo{"mfc1", InstructionType::Register}},
    {2, DisassemblyInfo{"cfc1", InstructionType::Register}},
    {4, DisassemblyInfo{"mtc1", InstructionType::Register}},
    {6, DisassemblyInfo{"ctc1", InstructionType::Register}},
    {8, DisassemblyInfo{"bc1", InstructionType::None}},
    {16, DisassemblyInfo{"fpu_s", InstructionType::None}},
    {20, DisassemblyInfo{"fpu_w", InstructionType::None}},
};

static std::map<int, DisassemblyInfo> bc1_table = {
    {0, DisassemblyInfo{"bc1f", InstructionType::Register}},
    {1, DisassemblyInfo{"bc1t", InstructionType::Register}},
    {2, DisassemblyInfo{"bc1fl", InstructionType::Register}},
    {3, DisassemblyInfo{"bc1tl", InstructionType::Register}},
};

static std::map<int, DisassemblyInfo> fpu_s_table = {
    {0, DisassemblyInfo{"add.s $fd, $fs, $ft", InstructionType::Register}},
    {1, DisassemblyInfo{"sub.s $fd, $fs, $ft", InstructionType::Register}},
    {5, DisassemblyInfo{"abs.s $fd, $fs", InstructionType::Register}},
    {6, DisassemblyInfo{"mov.s $fd, $fs", InstructionType::Register}},
    {7, DisassemblyInfo{"neg.s $fd, $fs", InstructionType::Register}},
    {24, DisassemblyInfo{"adda.s $fs, $ft", InstructionType::Register}},
    {25, DisassemblyInfo{"suba.s $fs, $ft", InstructionType::Register}},
    {28, DisassemblyInfo{"madd.s $fd, $fs, $ft", InstructionType::Register}},
    {40, DisassemblyInfo{"max.s $fd, $fs, $ft", InstructionType::Register}},
    {41, DisassemblyInfo{"min.s $fd, $fs, $ft", InstructionType::Register}},
    {48, DisassemblyInfo{"c.f.s, $fs, $ft", InstructionType::Register}},
    {50, DisassemblyInfo{"c.eq.s, $fs, $ft", InstructionType::Register}},
    {52, DisassemblyInfo{"c.lt.s, $fs, $ft", InstructionType::Register}},
    {54, DisassemblyInfo{"c.le.s, $fs, $ft", InstructionType::Register}},
};

static std::map<int, DisassemblyInfo> fpu_w_table = {
    // {2, DisassemblyInfo{"cfc2 $rt, $rd", InstructionType::Register}},
};

static std::map<int, DisassemblyInfo> cop2_table = {
    {2, DisassemblyInfo{"cfc2 $rt, $rd", InstructionType::Register}},
};

static std::map<int, DisassemblyInfo> tlb_table = {
    {2, DisassemblyInfo{"tlbwi", InstructionType::None}},
    {24, DisassemblyInfo{"eret", InstructionType::None}},
    {56, DisassemblyInfo{"ei", InstructionType::None}},
    {57, DisassemblyInfo{"di", InstructionType::None}},
};

static std::map<int, DisassemblyInfo> mmi_table = {
    {0, DisassemblyInfo{"madd $rd, $rs, $rt", InstructionType::Register}},
    {1, DisassemblyInfo{"maddu $rd, $rs, $rt", InstructionType::Register}},
    {4, DisassemblyInfo{"plzcw $rd, $rs", InstructionType::Register}},
    {8, DisassemblyInfo{"mmi0", InstructionType::None}},
    {9, DisassemblyInfo{"mmi2", InstructionType::None}},
    {16, DisassemblyInfo{"mfhi1 $rd", InstructionType::Register}},
    {17, DisassemblyInfo{"mthi1 $rs", InstructionType::Register}},
    {18, DisassemblyInfo{"mflo1 $rd", InstructionType::Register}},
    {19, DisassemblyInfo{"mtlo1 $rs", InstructionType::Register}},
    {24, DisassemblyInfo{"mult1 $rd, $rs, $rt", InstructionType::Register}},
    {25, DisassemblyInfo{"multu1 $rd, $rs, $rt", InstructionType::Register}},
    {26, DisassemblyInfo{"div1 $rs, $rt", InstructionType::Register}},
    {27, DisassemblyInfo{"divu1 $rs, $rt", InstructionType::Register}},
    {32, DisassemblyInfo{"madd1 $rd, $rs, $rt", InstructionType::Register}},
    {33, DisassemblyInfo{"maddu1 $rd, $rs, $rt", InstructionType::Register}},
    {40, DisassemblyInfo{"mmi1", InstructionType::None}},
    {41, DisassemblyInfo{"mmi3", InstructionType::None}},
};

static std::map<int, DisassemblyInfo> mmi0_table = {
    {0, DisassemblyInfo{"paddw $rd, $rs, $rt", InstructionType::Register}},
    {1, DisassemblyInfo{"psubw $rd, $rs, $rt", InstructionType::Register}},
    {2, DisassemblyInfo{"pcgtw $rd, $rs, $rt", InstructionType::Register}},
    {3, DisassemblyInfo{"pmaxw $rd, $rs, $rt", InstructionType::Register}},
    {4, DisassemblyInfo{"paddh $rd, $rs, $rt", InstructionType::Register}},
    {5, DisassemblyInfo{"psubh $rd, $rs, $rt", InstructionType::Register}},
    {6, DisassemblyInfo{"pcgth $rd, $rs, $rt", InstructionType::Register}},
    {7, DisassemblyInfo{"pmaxh $rd, $rs, $rt", InstructionType::Register}},
    {8, DisassemblyInfo{"paddb $rd, $rs, $rt", InstructionType::Register}},
    {9, DisassemblyInfo{"psubb $rd, $rs, $rt", InstructionType::Register}},
    {10, DisassemblyInfo{"pcgtb $rd, $rs, $rt", InstructionType::Register}},
    {16, DisassemblyInfo{"paddsw $rd, $rs, $rt", InstructionType::Register}},
    {17, DisassemblyInfo{"psubsw $rd, $rs, $rt", InstructionType::Register}},
    {20, DisassemblyInfo{"paddsh $rd, $rs, $rt", InstructionType::Register}},
    {21, DisassemblyInfo{"psubsh $rd, $rs, $rt", InstructionType::Register}},
    {24, DisassemblyInfo{"paddsb $rd, $rs, $rt", InstructionType::Register}},
    {25, DisassemblyInfo{"psubsb $rd, $rs, $rt", InstructionType::Register}},
};

static std::map<int, DisassemblyInfo> mmi1_table = {
    {1, DisassemblyInfo{"pabsw $rd, $rt", InstructionType::Register}},
    {2, DisassemblyInfo{"pceqw $rd, $rs, $rt", InstructionType::Register}},
    {3, DisassemblyInfo{"pminw $rd, $rs, $rt", InstructionType::Register}},
    {4, DisassemblyInfo{"padsbh $rd, $rs, $rt", InstructionType::Register}},
    {5, DisassemblyInfo{"pabsh $rd, $rt", InstructionType::Register}},
    {6, DisassemblyInfo{"pceqh $rd, $rs, $rt", InstructionType::Register}},
    {7, DisassemblyInfo{"pminh $rd, $rt", InstructionType::Register}},
    {10, DisassemblyInfo{"pceqb $rd, $rs, $rt", InstructionType::Register}},
    {16, DisassemblyInfo{"padduw $rd, $rs, $rt", InstructionType::Register}},
    {17, DisassemblyInfo{"psubuw $rd, $rs, $rt", InstructionType::Register}},
    {20, DisassemblyInfo{"padduh $rd, $rs, $rt", InstructionType::Register}},
    {21, DisassemblyInfo{"psubuh $rd, $rs, $rt", InstructionType::Register}},
    {24, DisassemblyInfo{"paddub $rd, $rs, $rt", InstructionType::Register}},
    {25, DisassemblyInfo{"psubub $rd, $rs, $rt", InstructionType::Register}},
};

static std::map<int, DisassemblyInfo> mmi2_table = {
    {0, DisassemblyInfo{"paddw $rd, $rs, $rt", InstructionType::Register}},
    {14, DisassemblyInfo{"pcpyld $rd, $rs, $rt", InstructionType::Register}},
    {18, DisassemblyInfo{"pand $rd, $rs, $rt", InstructionType::Register}},
    {19, DisassemblyInfo{"pxor $rd, $rs, $rt", InstructionType::Register}},
};

static std::map<int, DisassemblyInfo> mmi3_table = {
    {0, DisassemblyInfo{"pmadduw $rd, $rs, $rt", InstructionType::Register}},
    {14, DisassemblyInfo{"pcpyud $rd, $rs, $rt", InstructionType::Register}},
    {19, DisassemblyInfo{"pnor $rd, $rs, $rt", InstructionType::Register}},
    {27, DisassemblyInfo{"pcpyh $rd, $rt", InstructionType::Register}},
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

static std::string DisassembleImmediate(Instruction inst, u32 pc, std::string format) {
    std::string disassembled;
    u64 i = 0;

    while (i < format.length()) {
        if (format.compare(i, 3, "$rt") == 0) {
            disassembled += "$" + reg_names[inst.rt];
            i += 3;
        } else if (format.compare(i, 3, "$ft") == 0) {
            disassembled += "$f" + std::to_string(inst.rt);
            i += 3;
        } else if (format.compare(i, 3, "$rs") == 0) {
            disassembled += "$" + reg_names[inst.rs];
            i += 3;
        } else if (format.compare(i, 4, "$imm") == 0) {
            disassembled += ConvertHex<u16>(inst.imm);
            i += 4;
        } else if (format.compare(i, 7, "$offset") == 0) {
            disassembled += ConvertHex<u32>(pc + (inst.simm << 2) + 4);
            i += 7;
        } else {
            disassembled += format[i];
            i++;
        }
    }

    return disassembled;
}

static std::string DisassembleJump(Instruction inst, u32 pc, std::string format) {
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

static std::string DisassembleRegister(Instruction inst, u32 pc, std::string format) {
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
        } else if (format.compare(i, 3, "$fd") == 0) {
            disassembled += "$f" + std::to_string(inst.fd);
            i += 3;
        } else if (format.compare(i, 3, "$fs") == 0) {
            disassembled += "$f" + std::to_string(inst.fs);
            i += 3;
        } else if (format.compare(i, 3, "$ft") == 0) {
            disassembled += "$f" + std::to_string(inst.ft);
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

std::string DisassembleInstruction(Instruction inst, u32 pc) {
    std::string disassembled;

    DisassemblyInfo info = primary_table[inst.opcode];

    if (info.format.compare("secondary") == 0) {
        info = secondary_table[inst.func];
    } else if (info.format.compare("regimm") == 0) {
        info = regimm_table[inst.rt];
    } else if (info.format.compare("cop0") == 0) {
        info = cop0_table[inst.rs];

        if (info.format.compare("tlb") == 0) {
            info = tlb_table[inst.func];
        }
    } else if (info.format.compare("mmi") == 0) {
        info = mmi_table[inst.func];

        if (info.format.compare("mmi0") == 0) {
            info = mmi0_table[inst.imm5];
        } else if (info.format.compare("mmi1") == 0) {
            info = mmi1_table[inst.imm5];
        } else if (info.format.compare("mmi2") == 0) {
            info = mmi2_table[inst.imm5];
        } else if (info.format.compare("mmi3") == 0) {
            info = mmi3_table[inst.imm5];
        }
    } else if (info.format.compare("cop1") == 0) {
        info = cop1_table[inst.rs];

        if (info.format.compare("bc1") == 0) {
            info = bc1_table[inst.rt];
        } else if (info.format.compare("fpu_s") == 0) {
            info = fpu_s_table[inst.func];
        } else if (info.format.compare("fpu_w") == 0) {
            info = fpu_w_table[inst.func];
        }
    } else if (info.format.compare("cop2") == 0) {
        info = cop2_table[inst.rs];
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

std::string GetRegisterName(int reg) {
    return reg_names[reg];
}

} // namespace ee
