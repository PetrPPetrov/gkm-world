// Copyright 2018-2019 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <cstddef>

struct CellIndex
{
    std::int32_t x = 0;
    std::int32_t y = 0;

    CellIndex() : x(0), y(0)
    {
    }
    CellIndex(std::int32_t x_, std::int32_t y_) : x(x_), y(y_)
    {
    }
};

struct CellBox
{
    CellIndex min;
    CellIndex max;

    CellBox()
    {
    }
    CellBox(CellIndex min_, CellIndex max_) : min(min_), max(max_)
    {
    }
};
