// Copyright 2018-2019 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <memory>
#include "global_types.h"
#include "balance_tree/common.h"
#include "fast_index_map.h"

struct BalancerTreeInfo
{
    std::uint32_t token = 0;
    std::size_t level = 0;
    SquareCell bounding_box;
    bool leaf_node = true;
    std::array<std::uint32_t, CountOfChildren> children;
    std::array<std::uint32_t, 4 * (NEIGHBOR_COUNT_AT_SIDE + 1)> neighbors;
    std::uint32_t user_count = 0;
};

struct BalancerServerInfo
{
    typedef std::shared_ptr<BalancerServerInfo> Ptr;

    SquareCell bounding_box;
    std::uint32_t tree_root_token;
    std::uint32_t wait_info_for_token;

    Memory::FastIndexMap<BalancerTreeInfo> token_to_tree_node;
};
