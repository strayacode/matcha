#pragma once

#include <array>
#include "common/log.h"

template <typename T, int size>
class RingBuffer {
public:
    void Reset() {
        read_index = 0;
        write_index = 0;
    }

    void Push(T data) {
        buffer[write_index % size] = data;
        write_index = (write_index + 1) % size;
        capacity++;
    }

    T Pop() {
        T data = buffer[read_index % size];
        read_index = (read_index + 1) % size;
        capacity--;

        return data;
    }

    int Size() {
        return capacity;
    }

private:
    int read_index = 0;
    int write_index = 0;
    int capacity = 0;

    std::array<T, size> buffer;
};