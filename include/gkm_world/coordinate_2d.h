// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include "gkm_world/gkm_world.h"

struct Coordinate2D
{
    CoordinateType x = 0;
    CoordinateType y = 0;

    Coordinate2D()
    {
    }
    Coordinate2D(CoordinateType x_, CoordinateType y_) : x(x_), y(y_)
    {
    }
};

struct Box2D
{
    Coordinate2D min;
    Coordinate2D max;

    Box2D()
    {
    }
    Box2D(Coordinate2D min_, Coordinate2D max_) : min(min_), max(max_)
    {
    }
};

inline bool inside(const Box2D& box, Coordinate2D position)
{
    if (position.x >= box.min.x && position.x < box.max.x &&
        position.y >= box.min.y && position.y < box.max.y)
    {
        return true;
    }
    else
    {
        return false;
    }
}

inline bool inside(const Box2D& box, CoordinateType x_position, CoordinateType y_position)
{
    if (x_position >= box.min.x && x_position < box.max.x &&
        y_position >= box.min.y && y_position < box.max.y)
    {
        return true;
    }
    else
    {
        return false;
    }
}

struct Square2D
{
    Coordinate2D start;
    CoordinateType size = NODE_SIZE_MAX;

    Square2D()
    {
    }
    Square2D(Coordinate2D start_, CoordinateType size_) : start(start_), size(size_)
    {
    }
};

inline bool inside(const Square2D& square, Coordinate2D position)
{
    if (position.x >= square.start.x && position.x < square.start.x + square.size &&
        position.y >= square.start.y && position.y < square.start.y + square.size)
    {
        return true;
    }
    else
    {
        return false;
    }
}

inline bool inside(const Square2D& square, CoordinateType x_position, CoordinateType y_position)
{
    if (x_position >= square.start.x && x_position < square.start.x + square.size &&
        y_position >= square.start.y && y_position < square.start.y + square.size)
    {
        return true;
    }
    else
    {
        return false;
    }
}

inline Coordinate2D squareGlobalToLocal(const Coordinate2D& global_position, const Square2D& square)
{
    return Coordinate2D(global_position.x - square.start.x, global_position.y - square.start.y);
}

inline Coordinate2D squareLocalToGlobal(const Square2D& square, const Coordinate2D& local_position)
{
    return Coordinate2D(square.start.x + local_position.x, square.start.y + local_position.y);
}
