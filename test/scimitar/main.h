#pragma once
#include "include.h"
#include "typedef.h"

#include "../utils/crashhandler.h"

#include "Manager/GameManager.h"
#include "Manager/GamerProfile.h"
#include "Manager/CameraFX.h"

#include "World/World.h"

#include "Component/DamageComponent.h"
#include "Component/SkeletonComponent.h"
#include "Component/PawnInventoryComponent.h"
#include "Component/WeaponItemComponent.h"
#include "Component/WeaponComponent.h"

#include "Player/Controller.h"
#include "Player/Pawn.h"
#include "Player/Entity.h"

#include "Weapon/WeaponData.h"


namespace scimitar
{
    bool m_ScimitarInitialized{ false };

    void init()
    {
        CrashHandler::Enable();

        g_Offsets = std::make_shared<COffsets>();

        image_base = (uint64_t)GetModuleHandleA(NULL);

        // RESOLVE FUNCTIONS
        RESOLVE_FUNCTION(fn_GetBoneID,              0x58EBC0);
        RESOLVE_FUNCTION(fn_GetGlobalPosition,      0x5923A0);
        RESOLVE_FUNCTION(fn_GetMainGamerProfile,    0x1EB8B10);
        RESOLVE_FUNCTION(fn_Get_InGhostMode,        0x1D2AD40);
        RESOLVE_FUNCTION(fn_Set_InGhostMode,        0x1DDB5C0);

        // ADD OFFSETS
        g_Offsets->New(R"(R6GameManager::instance)",            0x79EFD88);
        g_Offsets->New(R"(R6GamerProfileManager::instance)",    0x79EFD78);
        g_Offsets->New(R"(CameraFX::instance)",                 0x7214CB0);

        // always have this at the bottom.
        m_ScimitarInitialized = true;
    }
    void main()
    {
        if (!m_ScimitarInitialized)
        {
            LOG("scimitar::main run before scimitar was initialized.");
        }
        R6GameManager* GameManager = R6GameManager::get();
        R6GamerProfile* GamerProfile = R6GamerProfile::GetMainGamerProfile();
    
        if (!GameManager || !GamerProfile)
            return;
    
        PlayerController* LocalPlayer = GamerProfile->m_BaseController();
        LOG("Saving local player controller @ %p", LocalPlayer);

        // Temporary handling
        while (true)
        {
            if (GetAsyncKeyState(VK_F1) & 1)
            {
                if (!valid_pointer(GameManager))
                    continue;

                if (!GameManager->m_IsPlaying())
                    continue;

                if (!valid_pointer(LocalPlayer))
                    continue;

                LocalPlayer->ChangeGhostMode();
            }
            if (GetAsyncKeyState(VK_F2) & 1)
            {
                if (!GameManager->m_IsPlaying())
                {
                    LOG("Skipping as we are in the menu!");
                    continue;
                }

                World* World = GameManager->m_World();
                if (!World)
                    continue;

                ubiArray<Entity**> Entities = World->m_Entities();
                if (!Entities.data)
                    continue;

                for (auto& Handle : Entities)
                {          
                    if (!Handle || !valid_pointer(Handle))
                        continue;

                    Entity* Entity = *Handle;
                    if (!Entity || !valid_pointer(Entity))
                        continue;

                    auto m_DamageComponent = Entity->GetComponent<DamageComponent*>(0x1F899234);
                    auto m_SkeletonComponent = Entity->GetComponent<SkeletonComponent*>(0x71FDA747);
                    auto m_Pawn = Entity->GetComponent<Pawn*>(0x6928D2C7);
                    auto m_PawnInventoryComponent = Entity->GetComponent<PawnInventoryComponent*>(0x8DCCE9A8);

                    if (!m_SkeletonComponent || !m_DamageComponent)
                        continue;

                    if (m_DamageComponent->m_Health() <= 0)
                        continue;


                    if (!valid_pointer(m_Pawn))
                        continue;

                    if (m_Pawn->m_Controller() == LocalPlayer)
                    {
                        auto PtrWeaponItemComponent = m_PawnInventoryComponent->m_CurrentWeaponItem();
                        if (!PtrWeaponItemComponent)
                            continue;

                        auto WeaponItemComponent = *PtrWeaponItemComponent;
                        if (!WeaponItemComponent)
                            continue;

                        auto WeaponComponent = WeaponItemComponent->m_WeaponComponent();
                        if (!WeaponComponent)
                            continue;

                        auto PtrWeaponData = WeaponComponent->m_WeaponData();
                        if (!PtrWeaponData)
                            continue;

                        auto WeaponData = *PtrWeaponData;
                        if (!WeaponData)
                            continue;

                        auto PtrAmmoWeaponData = WeaponData->m_AmmoWeaponData();
                        if (!PtrAmmoWeaponData)
                            continue;

                        auto AmmoWeaponData = *PtrAmmoWeaponData;
                        if (!AmmoWeaponData)
                            continue;

                        auto BaseWeaponSystem = WeaponComponent->m_WeaponSystem();
                        if (!BaseWeaponSystem)
                            continue;

                        LOG("local player (solo)\nController: %p\nPawn: %p\nEntity: %p\nInventory: %p\nWeaponItem: %p\nWeaponComponent: %p\nWeaponData: %p\nAmmoWeaponData: %p\nBaseWeaponSystem: %p",
                            LocalPlayer,
                            m_Pawn,
                            Entity,
                            m_PawnInventoryComponent,
                            WeaponItemComponent,
                            WeaponComponent, WeaponData,
                            AmmoWeaponData,
                            BaseWeaponSystem
                        );

                        AmmoWeaponData->m_NbFiredBulletsAtOnce() = 15;
                        AmmoWeaponData->m_UnlimitedAmmoInWeapon() = false;
                        BaseWeaponSystem->m_Recoil() = 0.f;

                        break;
                    }
                }

            }

            if (GetAsyncKeyState(VK_END) & 1)
                return;


            Sleep(10);
        }
    }

    void thread()
    {
        init();
        main();
    }
    /* 
        [List of stuff I have found but due to no IMGUI I will not implement. #Soon]

            ---------------- GHOST MODE ----------------
            Controller* Controller = *(Pawn + 0x4E8);
            PlayerController::Set_IsInGhostMode(Controller, bIsActive); // 0x1DDB5D0

            ---------------- BONES (NO GUI
            Vector4 BonePosition = m_SkeletonComponent->GetBonePosition(0x8023796D);
            if (!BonePosition.empty())
                printf("%p\n\tHEAD -> %.2f %.2f %.2f\n\tHEALTH -> %i\n", Entity, BonePosition.x, BonePosition.y, BonePosition.z, m_DamageComponent->m_Health());

        
        [List of stuff I have yet to fix that is causing crashes but I have yet to debug why.]

            SkeletonComponent->GetBonePosition(BoneID); 
                - Don't know WHY it crashes.

            controller == LocalPlayer 
                - Again, no clue why this crashes??? im comparing 2 addresses why tf is this crashing me???
                probaly some c++ thing im unaware of.
                - using a Health check as no entity will have 200 health other than local player when playing
                a solo match.
    */
}