#pragma once

#include "Component.h"

namespace scimitar
{
    class BoneHandle;

    FUNCTION(std::uint32_t, GetBoneID, BoneHandle*);
    FUNCTION(std::uint32_t, GetGlobalPosition, BoneHandle*, Vector4*);

    class BoneHandle {
    public:
        uint32_t BoneID() {
            return fn_GetBoneID(this);
        }
        Vector4 GlobalPosition() {
            Vector4 result;
            fn_GetGlobalPosition(this, &result);
            return result;
        }
    };


    class SkeletonComponent : public Component {
    public:
        MEMBER(m_BoneInstances, ubiArray<BoneHandle>, 0x108);

        Vector4 GetBonePosition(uint32_t boneID) {
            ubiArray<BoneHandle> BoneInstances = this->m_BoneInstances();
            for (int i = 0; i < BoneInstances.count; ++i)
            {
                auto Bone = BoneInstances.data + (0x10 * i);
                if (Bone->BoneID() == boneID)
                    return Bone->GlobalPosition();
            }

            return {};
        }
    };

}