// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include "gkm_world/game_logic.h"
#include "gkm_world/fast_index.h"

struct UnitLocationInfo {
    UnitLocationToken unit_location;
    KeyboardState state;
};

typedef Memory::FastIndexChain<UnitLocationInfo, 16> UnitList;
