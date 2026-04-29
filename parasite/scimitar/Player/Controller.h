#pragma once

#include "../../include.h"

namespace scimitar
{
    class PlayerController;

    FUNCTION(bool, Get_InGhostMode, PlayerController*);
    FUNCTION(bool, Set_InGhostMode, PlayerController*, bool);

    class PlayerController
    {
    public:
        void ChangeGhostMode()
        {
            if (!this)
                return;

            bool InGhostMode = fn_Get_InGhostMode(this);

            fn_Set_InGhostMode(this, !InGhostMode);
        }
    };
}