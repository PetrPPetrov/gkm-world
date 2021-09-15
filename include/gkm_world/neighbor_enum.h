// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <cstdint>

enum class ENeighborIndex : std::uint8_t {
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

inline unsigned toInteger(ENeighborIndex value) {
    return static_cast<unsigned>(value);
}

constexpr unsigned NeighborFirst = static_cast<unsigned>(ENeighborIndex::NeighborFirst);
constexpr unsigned NeighborLast = static_cast<unsigned>(ENeighborIndex::NeighborLast);
