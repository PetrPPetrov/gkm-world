// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <fstream>
#include <QImage>
#include <QString>
#include <QFileInfo>
#include <QVector3D>
#include "Eigen/Eigen"

typedef std::shared_ptr<QImage> ImagePtr;

inline Eigen::Vector3d to_eigen(const QVector3D& vector)
{
    return Eigen::Vector3d(vector.x(), vector.y(), vector.z());
}

inline QVector3D to_qt(const Eigen::Vector3d& vector)
{
    return QVector3D(vector.x(), vector.y(), vector.z());
}

inline bool fileExists(const std::string& filename)
{
    QFileInfo check_file(QString(filename.c_str()));
    if (check_file.exists() && check_file.isFile())
    {
        return true;
    }
    else
    {
        return false;
    }
}

// TODO:
// Introduce some settings dialog box
// Save settings to a project file
// Downsample the generated texture to a desired size (for instance, 1024x1024 or 2048x2048 or 4096x4096)
// Remove code duplication at build_mesh.cpp:getPixel() and Texture::getInterpolatedPixel()
// Improve nesting by using the shared edges
// Make nesting algorithm more deterministic or introduce complete another, deterministic nesting algorithm
// GUI tweaks
// Saving and restoring window layout
// Remove rotation parameter at all
// Switch to use turbojpeg and Texture class instead of QImage
