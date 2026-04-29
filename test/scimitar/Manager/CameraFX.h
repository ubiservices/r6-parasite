#pragma once

#include "../../include.h"

namespace scimitar
{
    class CameraFX
    {
    public:
        MEMBER(m_FOV, float, 0x18);
        MEMBER(m_Position, Vector4, 0x20);

        static CameraFX* get()
        {
            return *(CameraFX**)(image_base + g_Offsets->Get(R"(CameraFX::instance)"));
        }
    };
}