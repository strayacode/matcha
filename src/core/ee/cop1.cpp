#include <core/ee/cop1.h>

void EECOP1::Reset() {
    for (int i = 0; i < 32; i++) {
        fpr[i].u = 0;
    }
}

u32 EECOP1::GetReg(int reg) {
    return fpr[reg].u;
}

void EECOP1::SetReg(int reg, u32 data) {
    fpr[reg].u = data;
}