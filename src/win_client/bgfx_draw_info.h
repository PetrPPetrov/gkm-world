// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <cstdint>
#include <memory>
#include "bgfx_common.h"

struct BgfxDrawRefInfo {
    typedef std::shared_ptr<BgfxDrawRefInfo> Ptr;

    BgfxProgramPtr program;
    BgfxUniformPtr texture;
};

struct BgfxDrawInfo {
    //Eigen::Matrix3d transformation_matrix;
    std::uint64_t state = BGFX_STATE_DEFAULT;
    BgfxDrawRefInfo::Ptr ref_info;
};
