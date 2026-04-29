#pragma once

#include "../../include.h"
#include "../Player/Controller.h"

namespace scimitar
{
    class R6GamerProfile;
    FUNCTION(R6GamerProfile*, GetMainGamerProfile, std::uint64_t);

    class R6GamerProfile
    {
    public:
        MEMBER(m_BaseController, PlayerController*, 0x68);

        static R6GamerProfile* GetMainGamerProfile()
        {
            auto R6GamerProfileManager = *(std::uint64_t*)(image_base + g_Offsets->Get(R"(R6GamerProfileManager::instance)"));
            if (!R6GamerProfileManager)
                return 0;

            

            return fn_GetMainGamerProfile(R6GamerProfileManager);
        }
    };
}