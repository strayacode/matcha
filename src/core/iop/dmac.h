#pragma once

#include "common/types.h"

class IOPDMAC {
public:
    void reset();

    // dma priority/enable
    // used for the first 7 channels
    u32 dpcr;

    // dma priority/enable 2
    // used for the remaining channels
    u32 dpcr2;
private:

};