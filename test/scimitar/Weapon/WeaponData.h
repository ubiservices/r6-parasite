#pragma once

#include "../../include.h"

namespace scimitar
{
    class AmmoWeaponData
    {
    public:
        MEMBER(m_FireRate, std::uint32_t, 0x30);
        MEMBER(m_NbFiredBulletsAtOnce, std::uint32_t, 0x3C);
        MEMBER(m_UnlimitedAmmoInWeapon, bool, 0x69);
    };
    class AccuracyWeaponData
    {
    public:
        MEMBER(m_AccuracyConeInitialAngle, float, 0x20);
    };
    class WeaponData
    {
    public:
        MEMBER(m_AmmoWeaponData, AmmoWeaponData**, 0x150);
        MEMBER(m_AccuracyWeaponData, AccuracyWeaponData**, 0x158);
    };

}