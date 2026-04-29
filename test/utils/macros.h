#pragma once
#include "../include.h"

#define MEMBER(name, type, offset) \
    __forceinline type& name() const { \
        return *reinterpret_cast<type*>(reinterpret_cast<uintptr_t>(this) + offset); \
    } \
    __forceinline void name(type value) const { \
        *reinterpret_cast<type*>(reinterpret_cast<uintptr_t>(this) + offset) = value; \
    }

#define FUNCTION(type, name, ...) inline type(*fn_##name)(__VA_ARGS__)
#define RESOLVE_FUNCTION(name, offset)                          \
    if (!(offset)) return;                                      \
    *(std::uint64_t*)(&name) = image_base + (offset);

#ifdef _DEBUG
#define LOG(text, ...) printf(text, ##__VA_ARGS__); printf("\n");
#else
#define LOG(text, ...) ((void)0)
#endif