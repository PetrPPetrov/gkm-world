// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <cstdint>
#include "gkm_world/gkm_world.h"

enum class EChildIndex : std::uint8_t {
    ChildLowerLeft,
    ChildUpperLeft,
    ChildUpperRight,
    ChildLowerRight,
    ChildLast,
    ChildFirst = ChildLowerLeft,
    CountOfChildren = ChildLast
};

inline unsigned toInteger(EChildIndex value) {
    return static_cast<unsigned>(value);
}

constexpr unsigned CountOfChildren = static_cast<unsigned>(EChildIndex::CountOfChildren);
constexpr unsigned ChildFirst = static_cast<unsigned>(EChildIndex::ChildFirst);
constexpr unsigned ChildLast = static_cast<unsigned>(EChildIndex::ChildLast);
constexpr CoordinateType NEIGHBOR_COUNT_AT_SIDE = NODE_SIZE_MAX / NODE_SIZE_MIN;
