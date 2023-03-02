#pragma once

#include <cstdint>

using u8 = std::uint8_t;
using s8 = std::int8_t;
using u16 = std::uint16_t;
using s16 = std::int16_t;
using u32 = std::uint32_t;
using s32 = std::int32_t;
using u64 = std::uint64_t;
using s64 = std::int64_t;
using f32 = float;
using f64 = double;
using VirtualAddress = u32;

union u128 {
    struct {
        u64 lo;
        u64 hi;
    };

    u64 ud[2];
    u32 uw[4];

    u128() {
        lo = 0;
        hi = 0;
    }

    u128 inline operator |(u128 value) {
        u128 data;
        data.lo = lo | value.lo;
        data.hi = hi | value.hi;
        return data;
    }

    u128 inline operator &(u128 value) {
        u128 data;
        data.lo = lo & value.lo;
        data.hi = hi & value.hi;
        return data;
    }

    u128 inline operator ^(u128 value) {
        u128 data;
        data.lo = lo ^ value.lo;
        data.hi = hi ^ value.hi;
        return data;
    }

    u128 inline operator ~() {
        u128 data;
        data.lo = ~lo;
        data.hi = ~hi;
        return data;
    }

    u128& operator=(const int value) {
        uw[0] = value;
        return *this;
    }
};