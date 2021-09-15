// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <memory>
#include <cstdint>
#include "gkm_world/game_logic.h"

struct AnotherPlayer {
    typedef std::shared_ptr<AnotherPlayer> Ptr;

    UnitLocationToken player_info;
    //BlockChain<Self> display_chain;

    //AnotherPlayer() : display_chain(this)
    //{
    //}

    AnotherPlayer() = default;
    AnotherPlayer(const AnotherPlayer&) = delete;
    AnotherPlayer& operator=(const AnotherPlayer&) = delete;
};
