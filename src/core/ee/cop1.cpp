#include <algorithm>
#include "common/log.h"
#include "common/bits.h"
#include "core/ee/cop1.h"

namespace ee {

void COP1::Reset() {
    for (int i = 0; i < 32; i++) {
        fpr[i].u = 0;
    }

    accumulator.u = 0;
    fcr0.data = 0x2e00;
    fcr31.data = 0x1000001;
}

u32 COP1::GetReg(int reg) {
    return fpr[reg].u;
}

void COP1::SetReg(int reg, u32 value) {
    fpr[reg].u = value;
}

u32 COP1::GetControlReg(int reg) {
    switch (reg) {
    case 0:
        return fcr0.data;
    case 31:
        return fcr31.data | 0x1000001;
    default:
        common::Error("[ee::COP1] handle control reg %d", reg);
    }
    
    return 0;
}

void COP1::SetControlReg(int reg, u32 value) {
    switch (reg) {
    case 0:
        fcr0.data = value;
        break;
    case 31:
        fcr31.data = value | 0x1000001;
        break;
    default:
        common::Error("[ee::COP1] handle control reg %d", reg);
    }
}

// converts a u32 representation of a ps2 float
// to one that conforms to IEEE 754 floats
f32 COP1::AsFloat(u32 value) {
    // we must handle the cases when exponent == 255 or 0
    switch (value & 0x7f800000) {
    case 0:
        // denormals automatically get set to 0, while preserving their sign
        value &= 0x80000000;
        return common::BitCast<f32>(value);
    case 0x7f800000:
        // ps2 floats with the exponent as 255 can have numbers larger than what is possible
        // with IEEE 754, so we just set to the highest possible float
        value = (value & 0x80000000) | 0x7f7fffff;
        return common::BitCast<f32>(value);
    default:
        return common::BitCast<f32>(value);
    }
}

bool COP1::IsInfinity(const Register& fpr) {
    return (fpr.exponent == 0xff) && (fpr.fraction == 0);
}

void COP1::CheckOverflow(Register& fpr, bool set_flags) {
    if (IsInfinity(fpr)) {
        fpr.u = (fpr.u & 0x80000000) | 0x7f7fffff;
        
        if (set_flags) {
            fcr31.o = true;
            fcr31.so = true;
        }
    } else {
        if (set_flags) {
            fcr31.o = false;
        }   
    }
}

void COP1::CheckUnderflow(Register& fpr, bool set_flags) {
    if ((fpr.exponent == 0) && (fpr.fraction != 0)) {
        fpr.u &= 0x80000000;
        if (set_flags) {
            fcr31.u = true;
            fcr31.su = true;
        }
    } else {
        if (set_flags) {
            fcr31.u = false;
        }   
    }
}

void COP1::adda_s(Instruction inst) {
    accumulator.f = AsFloat(fpr[inst.fs].u) + AsFloat(fpr[inst.ft].u);
    CheckOverflow(accumulator, true);
    CheckUnderflow(accumulator, true);
}

void COP1::madd_s(Instruction inst) {
    fpr[inst.fd].f = AsFloat(accumulator.u) + (AsFloat(fpr[inst.fs].u) * AsFloat(fpr[inst.ft].u));
    CheckOverflow(fpr[inst.fd], true);
    CheckUnderflow(fpr[inst.fd], true);
}

void COP1::mov_s(Instruction inst) {
    fpr[inst.fd].u = fpr[inst.fs].u;
}

void COP1::abs_s(Instruction inst) {
    // forcibly make positive sign
    fpr[inst.fd].u = fpr[inst.fs].u;
    fpr[inst.fd].sign = 0;
    fcr31.o = false;
    fcr31.u = false;
}

void COP1::add_s(Instruction inst) {
    fpr[inst.fd].f = AsFloat(fpr[inst.fs].u) + AsFloat(fpr[inst.ft].u);
    CheckOverflow(fpr[inst.fd], true);
    CheckUnderflow(fpr[inst.fd], true);
}

void COP1::max_s(Instruction inst) {
    const auto lhs = fpr[inst.fs].s;
    const auto rhs = fpr[inst.ft].s;

    if (lhs < 0 && rhs < 0) {
        fpr[inst.fd].s = std::min<s32>(lhs, rhs);
    } else {
        fpr[inst.fd].s = std::max<s32>(lhs, rhs);
    }

    fcr31.o = false;
    fcr31.u = false;
}

void COP1::min_s(Instruction inst) {
    const auto lhs = fpr[inst.fs].s;
    const auto rhs = fpr[inst.ft].s;

    if (lhs < 0 && rhs < 0) {
        fpr[inst.fd].s = std::max<s32>(lhs, rhs);
    } else {
        fpr[inst.fd].s = std::min<s32>(lhs, rhs);
    }

    fcr31.o = false;
    fcr31.u = false;
}

void COP1::neg_s(Instruction inst) {
    fpr[inst.fd].f = -fpr[inst.fs].f;
    fcr31.o = false;
    fcr31.u = false;
}

void COP1::sub_s(Instruction inst) {
    fpr[inst.fd].f = AsFloat(fpr[inst.fs].u) - AsFloat(fpr[inst.ft].u);
    CheckOverflow(fpr[inst.fd], true);
    CheckUnderflow(fpr[inst.fd], true);
}

void COP1::suba_s(Instruction inst) {
    accumulator.f = AsFloat(fpr[inst.fs].u) - AsFloat(fpr[inst.ft].u);
    CheckOverflow(accumulator, true);
    CheckUnderflow(accumulator, true);
}

} // namespace ee