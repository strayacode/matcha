#pragma once

#include "common/types.h"
#include "common/memory.h"

namespace gs {

struct Page {
    void Reset() {
        for (int block = 0; block < 32; block++) {
            for (int pixel = 0; pixel < 256; pixel++) {
                blocks[block][pixel] = 0;
            }
        }
    }

    void WritePSMCT32Pixel(int x, int y, u32 value) {
        // get block coordinates
        int block_x = (x / 8) % 8;
        int block_y = (y / 8) % 4;

        constexpr static int block_configuration[4][8] = {
            {0, 1, 4, 5, 16, 17, 20, 21},
            {2, 3, 6, 7, 18, 19, 22, 23},
            {8, 9, 12, 13, 24, 25, 28, 29},
            {10, 11, 14, 15, 26, 27, 30, 31},
        };

        // get the block to write to based on configuration within page
        int block = block_configuration[block_y][block_x];

        constexpr static int pixel_configuration[2][8] = {
            {0, 1, 4, 5, 8, 9, 12, 13},
            {2, 3, 6, 7, 10, 11, 14, 15},
        };

        // find which column we want to write to
        int column = (y / 2) % 4;

        // get pixel coordinates
        int pixel_x = x % 8;
        int pixel_y = y % 2;

        int pixel = pixel_configuration[pixel_y][pixel_x];

        // each pixel is 32 bits, and each column is 16 pixels
        // hence each column is 64 bytes
        int offset = (column * 64) + (pixel * 4);
        common::Write<u32>(&blocks[block][offset], value);
    }

private:
    // each page contains 32 blocks, with each block being 256 bytes
    u8 blocks[32][256];
};

} // namespace gs