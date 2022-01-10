#pragma once

template <typename T, int size>
T sign_extend(T data) {
    struct {
        T data : size;
    } s;

    s.data = data;

    return s.data;
}