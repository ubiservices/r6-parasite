#pragma once

#include "../../include.h"
#include "../Component/Component.h"

namespace scimitar
{
    class Entity
    {
    public:
        MEMBER(m_Components, ubiArray<Component*>, 0xD8);

        template <typename T>
        T GetComponent(std::uint32_t class_id)
        {
            if (!this)
                return 0;

            for (auto& component : this->m_Components())
            {
                if (!component)
                    continue;

                std::uint64_t(*fn_descriptor)(Component * rcx);
                *(std::uint64_t*)(&fn_descriptor) = (std::uint64_t)(component->vftable[0x7]);

                auto descriptor = fn_descriptor(component);

                if (*(std::uint32_t*)(descriptor + 0x40) == class_id)
                    return reinterpret_cast<T>(component);

                continue;
            }

            return 0;
        }
    };
}