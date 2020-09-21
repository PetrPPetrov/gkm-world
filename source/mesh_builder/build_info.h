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

inline void loadEigenVector3d(Eigen::Vector3d& vector, std::ifstream& file_in)
{
    std::string buffer;
    std::getline(file_in, buffer);
    file_in >> vector.x();
    std::getline(file_in, buffer);
    std::getline(file_in, buffer);
    file_in >> vector.y();
    std::getline(file_in, buffer);
    std::getline(file_in, buffer);
    file_in >> vector.z();
    std::getline(file_in, buffer);
}

inline void saveEigenVector3d(const Eigen::Vector3d& vector, std::ofstream& file_out)
{
    file_out << "x\n";
    file_out << vector.x() << "\n";
    file_out << "y\n";
    file_out << vector.y() << "\n";
    file_out << "z\n";
    file_out << vector.z() << "\n";
}

inline void loadCameraInfo(CameraInfo::Ptr& camera_info, std::ifstream& file_in)
{
    std::string buffer;
    //std::getline(file_in, buffer);
    std::getline(file_in, buffer);
    file_in >> camera_info->photo_image_path;
    std::getline(file_in, buffer);
    std::getline(file_in, buffer);
    loadEigenVector3d(camera_info->viewer_pos, file_in);
    std::getline(file_in, buffer);
    loadEigenVector3d(camera_info->viewer_target, file_in);
    std::getline(file_in, buffer);
    loadEigenVector3d(camera_info->viewer_up, file_in);
    std::getline(file_in, buffer);
    file_in >> camera_info->rotation_radius;
    std::getline(file_in, buffer);
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

inline void loadBuildInfo(BuildInfo::Ptr& build_info, const std::string& file_name)
{
    std::ifstream file_in(file_name);
    std::string buffer;
    std::getline(file_in, buffer);
    std::getline(file_in, buffer);
    while (buffer == "camera_info")
    {
        auto new_camera_info = std::make_shared<CameraInfo>();
        loadCameraInfo(new_camera_info, file_in);
        build_info->cameras_info.push_back(new_camera_info);
        std::getline(file_in, buffer);
    }
}

inline void saveBuildInfo(const BuildInfo::Ptr& build_info, const std::string& file_name)
{
    std::ofstream file_out(file_name);
    file_out << "# Gkm-World Mesh Builder project file\n";
    for (auto camera_info : build_info->cameras_info)
    {
        saveCameraInfo(camera_info, file_out);
    }
}
