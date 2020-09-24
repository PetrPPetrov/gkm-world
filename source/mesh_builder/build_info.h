// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <list>
#include <memory>
#include <string>
#include <cstdint>
#include <fstream>
#include <QImage>
#include "common.h"

struct CameraInfo
{
    typedef std::shared_ptr<CameraInfo> Ptr;

    std::string photo_image_path;
    ImagePtr photo_image;
    Eigen::Vector3d viewer_pos;
    Eigen::Vector3d viewer_target;
    Eigen::Vector3d viewer_up;
    double rotation_radius;
};

struct BuildInfo
{
    typedef std::shared_ptr<BuildInfo> Ptr;

    std::vector<CameraInfo::Ptr> cameras_info;
};

inline void loadCameraInfo(CameraInfo::Ptr& camera_info, std::ifstream& file_in)
{
    loadToken("photo_image_path", file_in);
    file_in >> camera_info->photo_image_path;
    loadToken("viewer_pos", file_in);
    loadEigenVector3d(camera_info->viewer_pos, file_in);
    loadToken("viewer_target", file_in);
    loadEigenVector3d(camera_info->viewer_target, file_in);
    loadToken("viewer_up", file_in);
    loadEigenVector3d(camera_info->viewer_up, file_in);
    loadToken("rotation_radius", file_in);
    file_in >> camera_info->rotation_radius;
}

inline void saveCameraInfo(const CameraInfo::Ptr& camera_info, std::ofstream& file_out)
{
    file_out << "camera_info\n";
    file_out << "photo_image_path\n" << camera_info->photo_image_path << "\n";
    file_out << "viewer_pos\n";
    saveEigenVector3d(camera_info->viewer_pos, file_out);
    file_out << "viewer_target\n";
    saveEigenVector3d(camera_info->viewer_target, file_out);
    file_out << "viewer_up\n";
    saveEigenVector3d(camera_info->viewer_up, file_out);
    file_out << "rotation_radius\n" << camera_info->rotation_radius << "\n";
}
