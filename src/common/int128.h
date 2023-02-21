#pragma once

#include "common/types.h"

union u128 {
    struct {
        u64 lo;
        u64 hi;
    };

    u64 ud[2];
    u32 uw[4];

    u128 inline operator |(u128 value) {
        u128 data;
        data.lo = lo | value.lo;
        data.hi = hi | value.hi;
        return data;
    }

    u128& operator=(const int value) {
        uw[0] = value;
        return *this;
    }
};

union s128 {
    struct {
        s64 lo;
        s64 hi;
    };

    s64 sd[2];
};