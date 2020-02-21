// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <memory>
#include <cstdint>
#include "game_logic.h"
#include "block_chain.h"

struct AnotherPlayer
{
    typedef AnotherPlayer Self;
    typedef std::shared_ptr<Self> Ptr;

    PlayerUuidLocation player_info;
    ChainBlock<Self> display_chain;

    AnotherPlayer() : display_chain(this)
    {
    }

    AnotherPlayer(const AnotherPlayer&) = delete;
    AnotherPlayer& operator=(const AnotherPlayer&) = delete;
};
