// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <cstdint>

constexpr static int LINE_SET_VBO_MAX_VERTEX_COUNT = 16 * 1024;
constexpr static int PHOTO_MAX_VERTEX_COUNT = 6;

struct VertexPositionColor
{
    float x, y, z;
    std::uint32_t abgr;
};

struct VertexPositionTexCoord
{
    float x, y, z;
    float u, v;
};
