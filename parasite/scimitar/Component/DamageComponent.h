#pragma once

#include "Component.h"

namespace scimitar
{
    class DamageComponent : public Component
    {
    public:
        MEMBER(m_Health, std::uint16_t, 0x208);
    };
}