// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <memory>
#include <cstdint>

namespace ColorHasher
{
    std::uint32_t getColor(int id);
    int getRed(int id);
    int getGreen(int id);
    int getBlue(int id);
}
