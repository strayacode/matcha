#include "common/log.h"
#include "core/iop/cop0.h"

namespace iop {

void COP0::Reset() {
    status.data = 0;
    cause.data = 0;
    epc = 0;
    prid = 0x1f;
}

u32 COP0::GetReg(int reg) {
    switch (reg) {
    case 12:
        return status.data;
    case 13:
        return cause.data;
    case 14:
        return epc;
    case 15:
        return prid;
    default:
        common::Error("[iop::COP0] handle read r%d", reg);
    }

    return 0;
}

void COP0::SetReg(int reg, u32 value) {
    switch (reg) {
    case 3:
    case 5:
    case 6:
    case 7:
    case 9:
    case 11:
        break;
    case 12:
        status.data = value;
        break;
    case 13:
        common::Log("[iop::COP0] cause write %08x", value);
        break;
    case 14:
        epc = value;
        break;
    default:
        common::Error("[iop::COP0] handle write r%d = %08x", reg, value);
    }
}

} // namespace iop