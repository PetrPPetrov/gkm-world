// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <list>
#include <memory>
#include <QVector3D>

struct Box
{
    typedef std::shared_ptr<Box> Ptr;

    QVector3D position;
    QVector3D size;
};

struct AuxGeometry
{
    typedef std::shared_ptr<AuxGeometry> Ptr;

    std::list<Box::Ptr> boxes;
};
