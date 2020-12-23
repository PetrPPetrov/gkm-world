// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include "mesh_project.h"

void calculateVertexLinePoints(
    const Camera::Ptr& camera, int x, int y,
    Eigen::Vector3d& start, Eigen::Vector3d& finish);

void buildMesh(const MeshProject::Ptr& mesh_project);
