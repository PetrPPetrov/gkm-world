// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include "game_logic.h"
#include "block_chain.h"

struct UserLocation
{
    PlayerUuidLocation user_location;
    KeyboardState state;
};

typedef BlockChain<UserLocation> UserLocationBlockChain;
