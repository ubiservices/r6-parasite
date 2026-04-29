#pragma once

#include <cstdint>
#include <cstdio>
#include <Windows.h>
#include <memory>

#include "scimitar/typedef.h"
#include "utils/macros.h"

#include "offsets/offsets.h"

template <typename T>
inline bool valid_pointer(T address)
{
    __try
    {
        volatile auto result = *(void**)address;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return false;
    }
    return true;
}

inline std::uint64_t image_base;

inline std::shared_ptr<COffsets> g_Offsets;