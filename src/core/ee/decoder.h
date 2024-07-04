#pragma once

#include "core/ee/instruction.h"

namespace ee {

template <typename D, typename Handler = decltype(&D::illegal_instruction)>
struct Decoder {
    Decoder() {
        primary_table.fill(&D::illegal_instruction);
        secondary_table.fill(&D::illegal_instruction);
        regimm_table.fill(&D::illegal_instruction);
        cop0_table.fill(&D::illegal_instruction);
        cop1_table.fill(&D::illegal_instruction);
        bc1_table.fill(&D::illegal_instruction);
        fpu_s_table.fill(&D::illegal_instruction);
        fpu_w_table.fill(&D::illegal_instruction);
        cop2_table.fill(&D::stub_instruction);
        tlb_table.fill(&D::illegal_instruction);
        mmi_table.fill(&D::illegal_instruction);
        mmi0_table.fill(&D::illegal_instruction);
        mmi1_table.fill(&D::illegal_instruction);
        mmi2_table.fill(&D::illegal_instruction);
        mmi3_table.fill(&D::illegal_instruction);

        // primary instructions
        RegisterOpcode(&D::j, 2, InstructionType::Primary);
        RegisterOpcode(&D::jal, 3, InstructionType::Primary);
        RegisterOpcode(&D::beq, 4, InstructionType::Primary);
        RegisterOpcode(&D::bne, 5, InstructionType::Primary);
        RegisterOpcode(&D::blez, 6, InstructionType::Primary);
        RegisterOpcode(&D::bgtz, 7, InstructionType::Primary);
        RegisterOpcode(&D::addiu, 9, InstructionType::Primary);
        RegisterOpcode(&D::slti, 10, InstructionType::Primary);
        RegisterOpcode(&D::sltiu, 11, InstructionType::Primary);
        RegisterOpcode(&D::andi, 12, InstructionType::Primary);
        RegisterOpcode(&D::ori, 13, InstructionType::Primary);
        RegisterOpcode(&D::xori, 14, InstructionType::Primary);
        RegisterOpcode(&D::lui, 15, InstructionType::Primary);
        RegisterOpcode(&D::beql, 20, InstructionType::Primary);
        RegisterOpcode(&D::bnel, 21, InstructionType::Primary);
        RegisterOpcode(&D::blezl, 22, InstructionType::Primary);
        RegisterOpcode(&D::bgtzl, 23, InstructionType::Primary);
        RegisterOpcode(&D::daddiu, 25, InstructionType::Primary);
        RegisterOpcode(&D::ldl, 26, InstructionType::Primary);
        RegisterOpcode(&D::ldr, 27, InstructionType::Primary);
        RegisterOpcode(&D::lq, 30, InstructionType::Primary);
        RegisterOpcode(&D::sq, 31, InstructionType::Primary);
        RegisterOpcode(&D::lb, 32, InstructionType::Primary);
        RegisterOpcode(&D::lh, 33, InstructionType::Primary);
        RegisterOpcode(&D::lwl, 34, InstructionType::Primary);
        RegisterOpcode(&D::lw, 35, InstructionType::Primary);
        RegisterOpcode(&D::lbu, 36, InstructionType::Primary);
        RegisterOpcode(&D::lhu, 37, InstructionType::Primary);
        RegisterOpcode(&D::lwr, 38, InstructionType::Primary);
        RegisterOpcode(&D::lwu, 39, InstructionType::Primary);
        RegisterOpcode(&D::sb, 40, InstructionType::Primary);
        RegisterOpcode(&D::sh, 41, InstructionType::Primary);
        RegisterOpcode(&D::swl, 42, InstructionType::Primary);
        RegisterOpcode(&D::sw, 43, InstructionType::Primary);
        RegisterOpcode(&D::sdl, 44, InstructionType::Primary);
        RegisterOpcode(&D::sdr, 45, InstructionType::Primary);
        RegisterOpcode(&D::swr, 46, InstructionType::Primary);
        RegisterOpcode(&D::cache, 47, InstructionType::Primary);
        RegisterOpcode(&D::lwc1, 49, InstructionType::Primary);
        RegisterOpcode(&D::pref, 51, InstructionType::Primary);
        RegisterOpcode(&D::ld, 55, InstructionType::Primary);
        RegisterOpcode(&D::swc1, 57, InstructionType::Primary);
        RegisterOpcode(&D::sd, 63, InstructionType::Primary);

        // secondary instructions
        RegisterOpcode(&D::sll, 0, InstructionType::Secondary);
        RegisterOpcode(&D::srl, 2, InstructionType::Secondary);
        RegisterOpcode(&D::sra, 3, InstructionType::Secondary);
        RegisterOpcode(&D::sllv, 4, InstructionType::Secondary);
        RegisterOpcode(&D::srlv, 6, InstructionType::Secondary);
        RegisterOpcode(&D::srav, 7, InstructionType::Secondary);
        RegisterOpcode(&D::jr, 8, InstructionType::Secondary);
        RegisterOpcode(&D::jalr, 9, InstructionType::Secondary);
        RegisterOpcode(&D::movz, 10, InstructionType::Secondary);
        RegisterOpcode(&D::movn, 11, InstructionType::Secondary);
        RegisterOpcode(&D::syscall_exception, 12, InstructionType::Secondary);
        RegisterOpcode(&D::break_exception, 13, InstructionType::Secondary);
        RegisterOpcode(&D::sync, 15, InstructionType::Secondary);
        RegisterOpcode(&D::mfhi, 16, InstructionType::Secondary);
        RegisterOpcode(&D::mthi, 17, InstructionType::Secondary);
        RegisterOpcode(&D::mflo, 18, InstructionType::Secondary);
        RegisterOpcode(&D::mtlo, 19, InstructionType::Secondary);
        RegisterOpcode(&D::dsllv, 20, InstructionType::Secondary);
        RegisterOpcode(&D::dsrlv, 22, InstructionType::Secondary);
        RegisterOpcode(&D::dsrav, 23, InstructionType::Secondary);
        RegisterOpcode(&D::mult, 24, InstructionType::Secondary);
        RegisterOpcode(&D::multu, 25, InstructionType::Secondary);
        RegisterOpcode(&D::div, 26, InstructionType::Secondary);
        RegisterOpcode(&D::divu, 27, InstructionType::Secondary);
        RegisterOpcode(&D::addu, 33, InstructionType::Secondary);
        RegisterOpcode(&D::subu, 35, InstructionType::Secondary);
        RegisterOpcode(&D::andd, 36, InstructionType::Secondary);
        RegisterOpcode(&D::orr, 37, InstructionType::Secondary);
        RegisterOpcode(&D::xorr, 38, InstructionType::Secondary);
        RegisterOpcode(&D::nor, 39, InstructionType::Secondary);
        RegisterOpcode(&D::mfsa, 40, InstructionType::Secondary);
        RegisterOpcode(&D::mtsa, 41, InstructionType::Secondary);
        RegisterOpcode(&D::slt, 42, InstructionType::Secondary);
        RegisterOpcode(&D::sltu, 43, InstructionType::Secondary);
        RegisterOpcode(&D::daddu, 45, InstructionType::Secondary);
        RegisterOpcode(&D::dsubu, 47, InstructionType::Secondary);
        RegisterOpcode(&D::dsll, 56, InstructionType::Secondary);
        RegisterOpcode(&D::dsrl, 58, InstructionType::Secondary);
        RegisterOpcode(&D::dsra, 59, InstructionType::Secondary);
        RegisterOpcode(&D::dsll32, 60, InstructionType::Secondary);
        RegisterOpcode(&D::dsrl32, 62, InstructionType::Secondary);
        RegisterOpcode(&D::dsra32, 63, InstructionType::Secondary);

        // regimm instructions
        RegisterOpcode(&D::bltz, 0, InstructionType::RegImm);
        RegisterOpcode(&D::bgez, 1, InstructionType::RegImm);
        RegisterOpcode(&D::bltzl, 2, InstructionType::RegImm);
        RegisterOpcode(&D::bgezl, 3, InstructionType::RegImm);
        RegisterOpcode(&D::bltzal, 16, InstructionType::RegImm);
        RegisterOpcode(&D::bgezal, 17, InstructionType::RegImm);
        RegisterOpcode(&D::bltzall, 18, InstructionType::RegImm);
        RegisterOpcode(&D::bgezall, 19, InstructionType::RegImm);

        // cop0 instructions
        RegisterOpcode(&D::mfc0, 0, InstructionType::COP0);
        RegisterOpcode(&D::mtc0, 4, InstructionType::COP0);

        // cop1 instructions
        RegisterOpcode(&D::cfc1, 2, InstructionType::COP1);
        RegisterOpcode(&D::mtc1, 4, InstructionType::COP1);
        RegisterOpcode(&D::ctc1, 6, InstructionType::COP1);

        // bc1 instructions
        RegisterOpcode(&D::bc1f, 0, InstructionType::BC1);
        RegisterOpcode(&D::bc1t, 1, InstructionType::BC1);
        RegisterOpcode(&D::bc1fl, 2, InstructionType::BC1);
        RegisterOpcode(&D::bc1tl, 3, InstructionType::BC1);

        // fpu_s instructions
        RegisterOpcode(&D::add_s, 0, InstructionType::FPUS);
        RegisterOpcode(&D::sub_s, 1, InstructionType::FPUS);
        RegisterOpcode(&D::abs_s, 5, InstructionType::FPUS);
        RegisterOpcode(&D::mov_s, 6, InstructionType::FPUS);
        RegisterOpcode(&D::neg_s, 7, InstructionType::FPUS);
        RegisterOpcode(&D::adda_s, 24, InstructionType::FPUS);
        RegisterOpcode(&D::suba_s, 25, InstructionType::FPUS);
        RegisterOpcode(&D::madd_s, 28, InstructionType::FPUS);
        RegisterOpcode(&D::max_s, 40, InstructionType::FPUS);
        RegisterOpcode(&D::min_s, 41, InstructionType::FPUS);
        RegisterOpcode(&D::c_f_s, 48, InstructionType::FPUS);
        RegisterOpcode(&D::c_eq_s, 50, InstructionType::FPUS);

        // cop2 instructions
        RegisterOpcode(&D::cfc2, 2, InstructionType::COP2);
        RegisterOpcode(&D::ctc2, 6, InstructionType::COP2);

        // tlb instructions
        RegisterOpcode(&D::tlbwi, 2, InstructionType::TLB);
        RegisterOpcode(&D::eret, 24, InstructionType::TLB);
        RegisterOpcode(&D::ei, 56, InstructionType::TLB);
        RegisterOpcode(&D::di, 57, InstructionType::TLB);

        // mmi instructions
        RegisterOpcode(&D::madd, 0, InstructionType::MMI);
        RegisterOpcode(&D::maddu, 1, InstructionType::MMI);
        RegisterOpcode(&D::plzcw, 4, InstructionType::MMI);
        RegisterOpcode(&D::mfhi1, 16, InstructionType::MMI);
        RegisterOpcode(&D::mthi1, 17, InstructionType::MMI);
        RegisterOpcode(&D::mflo1, 18, InstructionType::MMI);
        RegisterOpcode(&D::mtlo1, 19, InstructionType::MMI);
        RegisterOpcode(&D::mult1, 24, InstructionType::MMI);
        RegisterOpcode(&D::multu1, 25, InstructionType::MMI);
        RegisterOpcode(&D::div1, 26, InstructionType::MMI);
        RegisterOpcode(&D::divu1, 27, InstructionType::MMI);
        RegisterOpcode(&D::madd1, 32, InstructionType::MMI);
        RegisterOpcode(&D::maddu1, 33, InstructionType::MMI);

        // mmi0 instructions
        RegisterOpcode(&D::paddw, 0, InstructionType::MMI0);
        RegisterOpcode(&D::psubw, 1, InstructionType::MMI0);
        RegisterOpcode(&D::pcgtw, 2, InstructionType::MMI0);
        RegisterOpcode(&D::pmaxw, 3, InstructionType::MMI0);
        RegisterOpcode(&D::paddh, 4, InstructionType::MMI0);
        RegisterOpcode(&D::psubh, 5, InstructionType::MMI0);
        RegisterOpcode(&D::pcgth, 6, InstructionType::MMI0);
        RegisterOpcode(&D::paddb, 8, InstructionType::MMI0);
        RegisterOpcode(&D::pmaxh, 7, InstructionType::MMI0);
        RegisterOpcode(&D::psubb, 9, InstructionType::MMI0);
        RegisterOpcode(&D::pcgtb, 10, InstructionType::MMI0);
        RegisterOpcode(&D::paddsw, 16, InstructionType::MMI0);
        RegisterOpcode(&D::psubsw, 17, InstructionType::MMI0);
        RegisterOpcode(&D::paddsh, 20, InstructionType::MMI0);
        RegisterOpcode(&D::psubsh, 21, InstructionType::MMI0);
        RegisterOpcode(&D::paddsb, 24, InstructionType::MMI0);
        RegisterOpcode(&D::psubsb, 25, InstructionType::MMI0);

        // mmi1 instructions
        RegisterOpcode(&D::pabsw, 1, InstructionType::MMI1);
        RegisterOpcode(&D::pceqw, 2, InstructionType::MMI1);
        RegisterOpcode(&D::pminw, 3, InstructionType::MMI1);
        RegisterOpcode(&D::padsbh, 4, InstructionType::MMI1);
        RegisterOpcode(&D::pabsh, 5, InstructionType::MMI1);
        RegisterOpcode(&D::pceqh, 6, InstructionType::MMI1);
        RegisterOpcode(&D::pminh, 7, InstructionType::MMI1);
        RegisterOpcode(&D::pceqb, 10, InstructionType::MMI1);
        RegisterOpcode(&D::padduw, 16, InstructionType::MMI1);
        RegisterOpcode(&D::psubuw, 17, InstructionType::MMI1);
        RegisterOpcode(&D::padduh, 20, InstructionType::MMI1);
        RegisterOpcode(&D::psubuh, 21, InstructionType::MMI1);
        RegisterOpcode(&D::paddub, 24, InstructionType::MMI1);
        RegisterOpcode(&D::psubub, 25, InstructionType::MMI1);

        // mmi2 instructions
        RegisterOpcode(&D::pcpyld, 14, InstructionType::MMI2);
        RegisterOpcode(&D::pand, 18, InstructionType::MMI2);
        RegisterOpcode(&D::pxor, 19, InstructionType::MMI2);

        // mmi3 instructions
        RegisterOpcode(&D::pcpyud, 14, InstructionType::MMI3);
        RegisterOpcode(&D::por, 18, InstructionType::MMI3);
        RegisterOpcode(&D::pnor, 19, InstructionType::MMI3);
        RegisterOpcode(&D::pcpyh, 27, InstructionType::MMI3);
    }

    Handler GetHandler(Instruction inst) {
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
            case 8:
                return bc1_table[inst.rt];
            case 16:
                return fpu_s_table[inst.func];
            case 20:
                return fpu_w_table[inst.func];
            }

            return cop1_table[inst.rs];
        case 18:
            return cop2_table[inst.rs];
        case 28:
            switch (inst.func) {
            case 8:
                return mmi0_table[inst.imm5];
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

private:
    enum class InstructionType {
        Primary,
        Secondary,
        RegImm,
        COP0,
        COP1,
        BC1,
        FPUS,
        FPUW,
        COP2,
        TLB,
        MMI,
        MMI0,
        MMI1,
        MMI2,
        MMI3,
    };

    void RegisterOpcode(Handler callback, int index, InstructionType type) {
        switch (type) {
        case InstructionType::Primary:
            primary_table[index] = callback;
            break;
        case InstructionType::Secondary:
            secondary_table[index] = callback;
            break;
        case InstructionType::RegImm:
            regimm_table[index] = callback;
            break;
        case InstructionType::COP0:
            cop0_table[index] = callback;
            break;
        case InstructionType::COP1:
            cop1_table[index] = callback;
            break;
        case InstructionType::BC1:
            bc1_table[index] = callback;
            break;
        case InstructionType::FPUS:
            fpu_s_table[index] = callback;
            break;
        case InstructionType::FPUW:
            fpu_w_table[index] = callback;
            break;
        case InstructionType::COP2:
            cop2_table[index] = callback;
            break;
        case InstructionType::TLB:
            tlb_table[index] = callback;
            break;
        case InstructionType::MMI:
            mmi_table[index] = callback;
            break;
        case InstructionType::MMI0:
            mmi0_table[index] = callback;
            break;
        case InstructionType::MMI1:
            mmi1_table[index] = callback;
            break;
        case InstructionType::MMI2:
            mmi2_table[index] = callback;
            break;
        case InstructionType::MMI3:
            mmi3_table[index] = callback;
            break;
        }
    }

    std::array<Handler, 64> primary_table;
    std::array<Handler, 64> secondary_table;
    std::array<Handler, 32> regimm_table;
    std::array<Handler, 32> cop0_table;
    std::array<Handler, 32> cop1_table;
    std::array<Handler, 32> bc1_table;
    std::array<Handler, 64> fpu_s_table;
    std::array<Handler, 64> fpu_w_table;
    std::array<Handler, 32> cop2_table;
    std::array<Handler, 64> tlb_table;
    std::array<Handler, 64> mmi_table;
    std::array<Handler, 32> mmi0_table;
    std::array<Handler, 32> mmi1_table;
    std::array<Handler, 32> mmi2_table;
    std::array<Handler, 32> mmi3_table;
};

} // namespace ee