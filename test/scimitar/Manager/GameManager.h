#pragma once

#include "../../include.h"
#include "../World/World.h"

namespace scimitar
{
    class R6GameManager
    {
    public:
        MEMBER(m_World, World*, 0x138);
        MEMBER(m_IsPlaying, bool, 0x207);

        static R6GameManager* get()
        {
            return *(R6GameManager**)(image_base + g_Offsets->Get(R"(R6GameManager::instance)"));
        }
    };
}