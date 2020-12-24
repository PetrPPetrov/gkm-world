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
// Add original images texture mode
// Add generated triangle images texture mode
// Debug rotated image case
// Do not process removed triangles
// Add noexcept keywords
// Add support of !valid_triangle TriangleTexture objects
// Reduce size of TriangleTexture::calculateDensity() method
// Correct texture coordinates after texture downsampling
// Improve nesting by using the shared edges
// Make nesting algorithm more deterministic or introduce complete another, deterministic nesting algorithm
// GUI tweaks
// Saving and restoring window layout
// Remove rotation parameter at all
// Switch to use turbojpeg and Texture class instead of QImage
