// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

enum EChildIndex : std::uint8_t
{
    ChildLowerLeft,
    ChildUpperLeft,
    ChildUpperRight,
    ChildLowerRight,
    ChildLast,
    ChildFirst = ChildLowerLeft,
    CountOfChildren = ChildLast
};

const std::uint32_t NEIGHBOR_COUNT_AT_SIDE = MAXIMAL_NODE_SIZE / MINIMAL_NODE_SIZE;
