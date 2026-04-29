#pragma once
#include "../../include.h"


namespace scimitar
{
    class Entity;

    class Component
    {
    public:
        std::uint64_t** vftable;
        MEMBER(m_Owner, Entity*, 0x18);
    };

}