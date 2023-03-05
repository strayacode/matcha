#pragma once

namespace gs {

struct Page {
    void Reset() {
        for (int block = 0; block < 32; block++) {
            for (int pixel = 0; pixel < 256; pixel++) {
                blocks[block][pixel] = 0;
            }
        }
    }

private:
    // each page contains 32 blocks, with each block being 256 bytes
    u8 blocks[32][256];
};

} // namespace gs