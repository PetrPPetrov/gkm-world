// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include "common.h"
#include "mesh_project.h"

typedef Eigen::Matrix<std::uint32_t, Triangle::vertex_count, 1> Vector3u;

struct Mesh
{
    typedef std::shared_ptr<Mesh> Ptr;

    std::vector<Eigen::Vector3d> vertices;
    std::vector<size_t> old_to_new_vertex_id_map;
    std::vector<size_t> new_to_old_vertex_id_map;

    std::vector<Vector3u> triangles;
    //std::vector<size_t> old_to_new_triangle_id_map;
};
