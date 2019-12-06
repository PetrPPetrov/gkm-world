// Copyright 2018-2019 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <cstdint>
#include "global_parameters.h"

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

inline bool inside(const CellBox& cell, double x_pos, double y_pos)
{
    if (x_pos >= cell.min.x * CELL_SIZE && x_pos < cell.max.x * CELL_SIZE &&
        y_pos >= cell.min.y * CELL_SIZE && y_pos < cell.max.y * CELL_SIZE)
    {
        return true;
    }
    else
    {
        return false;
    }
}

struct SquareCell
{
    CellIndex start;
    std::int32_t size = MAXIMAL_NODE_SIZE;

    SquareCell()
    {
    }
    SquareCell(CellIndex start_, std::int32_t size_) : start(start_), size(size_)
    {
    }
};

inline bool inside(const SquareCell& cell, double x_pos, double y_pos)
{
    if (x_pos >= cell.start.x * CELL_SIZE && x_pos < (cell.start.x + cell.size) * CELL_SIZE &&
        y_pos >= cell.start.y * CELL_SIZE && y_pos < (cell.start.y + cell.size) * CELL_SIZE)
    {
        return true;
    }
    else
    {
        return false;
    }
}

inline CellIndex cellGlobalToLocal(CellIndex global, const SquareCell& cell)
{
    return CellIndex(global.x - cell.start.x, global.y - cell.start.y);
}

inline CellIndex cellLocalToGlobal(const SquareCell& cell, CellIndex local)
{
    return CellIndex(cell.start.x + local.x, cell.start.y + local.y);
}
