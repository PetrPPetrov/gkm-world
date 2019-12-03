// Copyright 2018-2019 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include "bounding_box.h"
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

// |--------|--------|
// | first  | second |
// |--------|--------|
// | fourth | third  |
// |--------|--------|
// TODO: Revise this
inline ENeighborIndex getNeihgborByPosition(double x_pos, double y_pos, const box2i_t& bounding_box)
{
    const std::int32_t bb_min_x_pos = bounding_box.min_corner().get<0>();
    const std::int32_t bb_min_y_pos = bounding_box.min_corner().get<1>();
    const std::int32_t bb_max_x_pos = bounding_box.max_corner().get<0>();
    const std::int32_t bb_max_y_pos = bounding_box.max_corner().get<1>();
    const std::int32_t half_bb_max_x_pos = bounding_box.max_corner().get<0>() / 2;
    const std::int32_t half_bb_max_y_pos = bounding_box.max_corner().get<1>() / 2;

    // In the first
    if ((x_pos < half_bb_max_x_pos) && (y_pos > half_bb_max_y_pos))
    {
        if ((x_pos < bb_min_x_pos + NOTIFY_DISTANCE) && (y_pos < bb_max_y_pos - NOTIFY_DISTANCE))
            return NeighborLeftTop;
        if ((x_pos < bb_min_x_pos + NOTIFY_DISTANCE) && (y_pos > bb_max_y_pos - NOTIFY_DISTANCE))
            return NeighborUpperLeftCorner;
        if ((x_pos > bb_min_x_pos + NOTIFY_DISTANCE) && (y_pos > bb_max_y_pos - NOTIFY_DISTANCE))
            return NeighborUpperLeft;
    }

    // In the second
    if ((x_pos > half_bb_max_x_pos) && (y_pos > half_bb_max_y_pos))
    {
        if ((x_pos < bb_max_x_pos - NOTIFY_DISTANCE) && (y_pos > bb_max_y_pos - NOTIFY_DISTANCE))
            return NeighborUpperRight;
        if ((x_pos > bb_max_x_pos - NOTIFY_DISTANCE) && (y_pos > bb_max_y_pos - NOTIFY_DISTANCE))
            return NeighborUpperRightCorner;
        if ((x_pos > bb_max_x_pos - NOTIFY_DISTANCE) && (y_pos < bb_max_y_pos - NOTIFY_DISTANCE))
            return NeighborRightTop;
    }

    // In the third
    if ((x_pos > half_bb_max_x_pos) && (y_pos < half_bb_max_y_pos))
    {
        if ((x_pos > bb_max_x_pos - NOTIFY_DISTANCE) && (y_pos > bb_min_y_pos + NOTIFY_DISTANCE))
            return NeighborRightBottom;
        if ((x_pos > bb_max_x_pos - NOTIFY_DISTANCE) && (y_pos < bb_min_y_pos + NOTIFY_DISTANCE))
            return NeighborLowerRightCorner;
        if ((x_pos < bb_max_x_pos - NOTIFY_DISTANCE) && (y_pos < bb_min_y_pos + NOTIFY_DISTANCE))
            return NeighborLowerRight;
    }

    // In the fourth
    if ((x_pos < half_bb_max_x_pos) && (y_pos < half_bb_max_y_pos))
    {
        if ((x_pos > bb_min_x_pos + NOTIFY_DISTANCE) && (y_pos < bb_min_y_pos + NOTIFY_DISTANCE))
            return NeighborLowerLeft;
        if ((x_pos < bb_min_x_pos + NOTIFY_DISTANCE) && (y_pos < bb_min_y_pos + NOTIFY_DISTANCE))
            return NeighborLowerLeftCorner;
        if ((x_pos < bb_min_x_pos + NOTIFY_DISTANCE) && (y_pos > bb_min_y_pos + NOTIFY_DISTANCE))
            return NeighborLeftBottom;
    }

    // Neighbor not found
    assert(false);
}
