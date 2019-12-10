// Copyright 2018-2019 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <memory>
#include "global_types.h"

struct BalancerServerInfo
{
    typedef std::shared_ptr<BalancerServerInfo> Ptr;

    SquareCell bounding_box;
};
