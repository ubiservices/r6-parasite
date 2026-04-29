#pragma once

#include "../../include.h"
#include "Controller.h"

namespace scimitar
{
    class Pawn
    {
    public:
        MEMBER(m_Controller, PlayerController*, 0x4E8);
    };
}