// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include "main_window.h"
#include "build_mesh.h"
#include "mesh_builder.h"

MeshBuilder::MeshBuilder(const MeshProject::Ptr& mesh_project_) : mesh_project(mesh_project_)
{
    moveToThread(&working_thread);
    connect(&working_thread, SIGNAL(started()), this, SLOT(onThreadStart()));
    working_thread.start();
}

void MeshBuilder::onThreadStart()
{
    buildMesh(mesh_project);
}
