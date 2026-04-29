#pragma once

template<typename T>
struct ubiArray
{
    T* data;
    uint16_t count;
    uint16_t capacity;

    T* begin() { return data; }
    T* end() { return data + count; }
};

struct Vector4 {
    float x, y, z, w;

    bool empty()
    {
        return (x == 0 && y == 0 && z == 0 && w == 0);
    }
};