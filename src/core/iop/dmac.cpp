#include "core/iop/dmac.h"

void IOPDMAC::Reset() {
    dpcr = 0x07777777;
    dpcr2 = 0x07777777;
    dicr = 0;
    dicr2 = 0;
}