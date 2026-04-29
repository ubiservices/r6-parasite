#pragma once

#include "../../include.h"
#include "../Player/Entity.h"

namespace scimitar
{
    class World
    {
    public:
        MEMBER(m_Entities, ubiArray<Entity**>, 0x458);
    };
}