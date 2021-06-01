// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <memory>
#include <vector>
#include "pool.h"
#include "protocol.h"

namespace Packet
{
    template<std::size_t Capacity>
    class Pool : public Memory::Pool<void, Capacity, MAX_SIZE>
    {
    };
}
