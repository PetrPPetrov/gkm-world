// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include "global_types.h"
#include "global_parameters.h"

enum ENeighborIndex : std::uint8_t
{
    NeighborUpperLeftCorner,
    NeighborUpperLeft,
    NeighborUpperRight,
    NeighborUpperRightCorner,
    NeighborRightTop,
    NeighborRightBottom,
    NeighborLowerRightCorner,
    NeighborLowerRight,
    NeighborLowerLeft,
    NeighborLowerLeftCorner,
    NeighborLeftBottom,
    NeighborLeftTop,
    NeighborLast,
    NeighborFirst = NeighborUpperLeftCorner
};
