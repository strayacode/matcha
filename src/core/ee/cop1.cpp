#include "core/ee/cop1.h"

namespace ee {

void COP1::Reset() {
    for (int i = 0; i < 32; i++) {
        fpr[i].u = 0;
        control[i] = 0;
    }

    accumulator.u = 0;
}

u32 COP1::GetReg(int reg) {
    return fpr[reg].u;
}

void COP1::SetReg(int reg, u32 data) {
    fpr[reg].u = data;
}

u32 COP1::GetControlReg(int reg) {
    return control[reg];
}

void COP1::SetControlReg(int reg, u32 data) {
    control[reg] = data;
}

// converts a u32 representation of a ps2 float
// to one that conforms to IEEE 754 floats
f32 COP1::AsFloat(u32 value) {
    // we must handle the cases when exponent == 255 or 0
    switch (value & 0x7F800000) {
    case 0:
        // denormals automatically get set to 0
        value = 0;
        return static_cast<float>(value);
    case 0x7F800000:
        // ps2 floats with the exponent as 255 can have numbers larger than what is possible
        // with IEEE 754, so we just set to the highest possible float
        value = (value & 0x80000000) | 0x7F7FFFFF;
        return static_cast<float>(value);
    default:
        return static_cast<float>(value);
    }
}

} // namespace ee