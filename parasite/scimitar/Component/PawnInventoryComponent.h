#pragma once

#include "Component.h"
#include "WeaponItemComponent.h"

namespace scimitar
{
    class PawnInventoryComponent : public Component
    {
    public:
        MEMBER(m_CurrentWeaponItem, WeaponItemComponent**, 0x3F0);
    };
}