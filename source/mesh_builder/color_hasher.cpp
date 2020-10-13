// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include "color_hasher.h"

constexpr static std::uint32_t g_colors[] =
{
    0xff000000,
    0xff00ff00,
    0xff0000ff,
    0xffff0000,
    0xff01fffe,
    0xffffa6fe,
    0xffffdb66,
    0xff006401,
    0xff010067,
    0xff95003a,
    0xff007db5,
    0xffff00f6,
    0xffffeee8,
    0xff774d00,
    0xff90fb92,
    0xff0076ff,
    0xffd5ff00,
    0xffff937e,
    0xff6a826c,
    0xffff029d,
    0xfffe8900,
    0xff7a4782,
    0xff7e2dd2,
    0xff85a900,
    0xffff0056,
    0xffa42400,
    0xff00ae7e,
    0xff683d3b,
    0xffbdc6ff,
    0xff263400,
    0xffbdd393,
    0xff00b917,
    0xff9e008e,
    0xff001544,
    0xffc28c9f,
    0xffff74a3,
    0xff01d0ff,
    0xff004754,
    0xffe56ffe,
    0xff788231,
    0xff0e4ca1,
    0xff91d0cb,
    0xffbe9970,
    0xff968ae8,
    0xffbb8800,
    0xff43002c,
    0xffdeff74,
    0xff00ffc6,
    0xffffe502,
    0xff620e00,
    0xff008f9c,
    0xff98ff52,
    0xff7544b1,
    0xffb500ff,
    0xff00ff78,
    0xffff6e41,
    0xff005f39,
    0xff6b6882,
    0xff5fad4e,
    0xffa75740,
    0xffa5ffd2,
    0xffffb167,
    0xff009bff,
    0xffe85ebe
};
constexpr size_t g_colors_size = sizeof(g_colors) / sizeof(g_colors[0]);

namespace ColorHasher
{
    std::uint32_t getColor(int id)
    {
        return g_colors[id % g_colors_size];
    }

    int getRed(int id)
    {
        std::uint32_t color = getColor(id);
        return static_cast<int>(color & 0xff);
    }

    int getGreen(int id)
    {
        std::uint32_t color = getColor(id);
        return static_cast<int>((color & 0xff00) >> 8);
    }

    int getBlue(int id)
    {
        std::uint32_t color = getColor(id);
        return static_cast<int>((color & 0xff0000) >> 16);
    }
}
