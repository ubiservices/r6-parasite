#pragma once

#include "Component.h"
#include "WeaponComponent.h"

namespace scimitar
{
    class WeaponItemComponent
    {
    public:
        MEMBER(m_WeaponComponent, WeaponComponent*, 0x128);
    };
}