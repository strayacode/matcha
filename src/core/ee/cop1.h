#pragma once

#include "common/types.h"
#include "common/log.h"
#include "core/ee/instruction.h"

namespace ee {

class COP1 {
public:
    void Reset();

    u32 GetReg(int reg);
    void SetReg(int reg, u32 value);
    u32 GetControlReg(int reg);
    void SetControlReg(int reg, u32 value);
    f32 AsFloat(u32 value);

    bool condition() const { return fcr31.c; }

    void adda_s(Instruction inst);
    void madd_s(Instruction inst);
    void mov_s(Instruction inst);
    void abs_s(Instruction inst);
    void add_s(Instruction inst);
    void max_s(Instruction inst);
    void min_s(Instruction inst);
    void neg_s(Instruction inst);
    void sub_s(Instruction inst);
    void suba_s(Instruction inst);
    void c_eq_s(Instruction inst);
    void c_f_s();

private:
    union Float {
        struct {
            u32 fraction : 23;
            u32 exponent : 8;
            u32 sign : 1;
        };

        f32 f;
        u32 u;
        s32 s;

        static constexpr u32 MAX_POSITIVE = 0x7fffffff;
        static constexpr u32 MAX_NEGATIVE = 0xffffffff;
        static constexpr u32 POSITIVE_INFINITY = 0x7f800000;
        static constexpr u32 NEGATIVE_INFINITY = 0x7f800000;

        bool is_abnormal() {
            return u == MAX_POSITIVE || u == MAX_NEGATIVE || u == POSITIVE_INFINITY || u == NEGATIVE_INFINITY;
        }
    };

    union FCR0 {
        struct {
            u32 rev : 8;
            u32 imp : 8;
            u32 : 16;
        };

        u32 data;
    };

    union FCR31 {
        struct {
            u32: 3;
            bool su : 1;
            bool so : 1;
            bool sd : 1;
            bool si : 1;
            u32 : 7;
            bool u : 1;
            bool o : 1;
            bool d : 1;
            bool i : 1;
            u32 : 5;
            bool c : 1;
            u32 : 8;
        };

        u32 data;
    };

    bool IsInfinity(const Float& fpr);
    void CheckOverflow(Float& fpr, bool set_flags);
    void CheckUnderflow(Float& fpr, bool set_flags);

    Float fpr[32];
    Float accumulator;
    FCR0 fcr0;
    FCR31 fcr31;
};

} // namespace ee