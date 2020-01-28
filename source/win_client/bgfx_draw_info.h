// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <memory>
#include "bgfx_common.h"
#include "Eigen/Eigen"

struct BgfxDrawRefInfo
{
    typedef std::shared_ptr<BgfxDrawRefInfo> Ptr;

    bgfx_program_ptr program;
};

struct BgfxDrawInfo
{
    Eigen::Matrix3d transformation_matrix;
    uint64_t state = BGFX_STATE_DEFAULT;
    BgfxDrawRefInfo::Ptr ref_info;
};
