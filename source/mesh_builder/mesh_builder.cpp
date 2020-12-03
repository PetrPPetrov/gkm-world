// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include "main_window.h"
#include "build_mesh.h"
#include "mesh_builder.h"
#include "task.h"

MeshBuilder::MeshBuilder(const MeshProject::Ptr& mesh_project_) : QObject(nullptr), mesh_project(mesh_project_)
{
    moveToThread(&working_thread);
    connect(&working_thread, SIGNAL(started()), this, SLOT(onThreadStart()));
    connect(&working_thread, SIGNAL(finished()), this, SLOT(onThreadFinish()));
    ProgressCalculator::getInstance().start();
    working_thread.start();
}

void MeshBuilder::onThreadStart()
{
    try
    {
        buildMesh(mesh_project);
        g_main_window->setMeshBuildingProgressStatus("Done");
    }
    catch (const AbortException&)
    {
        aborted = true;
    }
    working_thread.quit();
}

void MeshBuilder::onThreadFinish()
{
    if (aborted)
    {
        g_main_window->meshBuildingAborted();
    }
    else
    {
        g_main_window->meshBuildingFinished();
    }
}
