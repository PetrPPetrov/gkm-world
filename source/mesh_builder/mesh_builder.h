// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <QObject>
#include <QThread>
#include <QTimer>
#include <QMutex>
#include "mesh_project.h"

class MeshBuilder : public QObject
{
    Q_OBJECT

public:
    MeshBuilder(const MeshProject::Ptr& mesh_project);

    QMutex abort_flag_locker;
    bool abort_flag = false;

private slots:
    void onThreadStart();

private:
    MeshProject::Ptr mesh_project;
    QThread working_thread;
};
