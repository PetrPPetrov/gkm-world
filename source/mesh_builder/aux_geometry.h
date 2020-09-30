// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <list>
#include <memory>
#include <QVector3D>
#include "common.h"

struct AuxGeometryBox
{
    typedef std::shared_ptr<AuxGeometryBox> Ptr;

    int id = 0;
    QVector3D position;
    QVector3D size;
};

struct AuxGeometry
{
    typedef std::shared_ptr<AuxGeometry> Ptr;

    std::list<AuxGeometryBox::Ptr> boxes;
};

inline void loadAuxGeometryBox(AuxGeometryBox::Ptr& box, std::ifstream& file_in)
{
    loadToken("id", file_in);
    file_in >> box->id;
    loadToken("position", file_in);
    loadQtVector3d(box->position, file_in);
    loadToken("size", file_in);
    loadQtVector3d(box->size, file_in);
}

inline void saveAuxGeometryBox(const AuxGeometryBox::Ptr& box, std::ofstream& file_out)
{
    file_out << "aux_geometry_box\n";
    file_out << "id\n" << box->id << "\n";
    file_out << "position\n";
    saveQtVector3d(box->position, file_out);
    file_out << "size\n";
    saveQtVector3d(box->size, file_out);
}
