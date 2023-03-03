#pragma once

#include <array>
#include "common/log.h"

namespace common {

template <typename InnerT, int size>
class Queue {
public:
    void Reset() {
        read_index = 0;
        write_index = 0;
        length = 0;
        buffer.fill(0);
    }

    template <typename T>
    void Push(T value) {
        static_assert(sizeof(T) >= sizeof(InnerT));
        static_assert(sizeof(T) <= (sizeof(InnerT) * size));
        int ratio = sizeof(T) / sizeof(InnerT);
        InnerT* data = reinterpret_cast<InnerT*>(&value);

        for (int i = 0; i < ratio; i++) {
            buffer[write_index % size] = data[i];
            write_index = (write_index + 1) % size;
            length++;
        }
    }

    template <typename T>
    T Pop() {
        static_assert(sizeof(T) >= sizeof(InnerT));
        static_assert(sizeof(T) <= (sizeof(InnerT) * size));
        int ratio = sizeof(T) / sizeof(InnerT);
        T value;
        InnerT* data = reinterpret_cast<InnerT*>(&value);

        for (int i = 0; i < ratio; i++) {
            data[i] = buffer[read_index % size];
            read_index = (read_index + 1) % size;
            length--;
        }

        return value;
    }

    int GetLength() {
        return length;
    }

    int GetSize() {
        return size;
    }

    bool Empty() {
        return length == 0;
    }

private:
    int read_index = 0;
    int write_index = 0;
    int length = 0;

    std::array<InnerT, size> buffer;
};

} // namespace common