// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include "vbo.h"
#include "mesh_project.h"

void fillLineSet(
    std::vector<VertexPositionColor>& vbo,
    const MeshProject::Ptr& mesh_project,
    const CameraInfo::Ptr cur_camera,
    int& aux_geom_line_set_vbo_size,
    int& hub_points_line_set_vbo_size,
    int photo_x_low, int photo_y_low
);
