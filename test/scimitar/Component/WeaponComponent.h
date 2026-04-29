#pragma once

#include "Component.h"
#include "../Weapon/WeaponData.h"
#include "../Weapon/BaseWeaponSystem.h"

namespace scimitar
{
    class WeaponComponent : public Component
    {
    public:
        MEMBER(m_WeaponData, WeaponData**, 0x1D8);
        MEMBER(m_WeaponSystem, BaseWeaponSystem*, 0x220);

        bool bIsHidden;
    };

    WeaponComponent* m_CurrentWeapon = nullptr;
}