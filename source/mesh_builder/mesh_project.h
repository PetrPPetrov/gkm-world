// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <memory>
#include <string>
#include <cstdint>
#include <fstream>
#include "common.h"
#include "aux_geometry.h"
#include "build_info.h"

struct MeshProject
{
    typedef std::shared_ptr<MeshProject> Ptr;

    AuxGeometry::Ptr aux_geometry = std::make_shared<AuxGeometry>();
    BuildInfo::Ptr build_info = std::make_shared<BuildInfo>();
    std::string file_name;
    std::string output_file_name;
    bool dirty = false;
};

inline void linkProject(MeshProject::Ptr& project)
{
    for (auto& vertex : project->build_info->vertices)
    {
        for (auto vertex_position : vertex->positions)
        {
            if (vertex_position->camera_index < project->build_info->cameras_info.size())
            {
                project->build_info->cameras_info[vertex_position->camera_index]->vertex_positions.push_back(vertex_position);
            }
        }
    }
}

inline void loadMeshProject(MeshProject::Ptr& project, const std::string& file_name)
{
    project = std::make_shared<MeshProject>();
    std::ifstream file_in(file_name);
    loadToken("# Gkm-World Mesh Builder project file", file_in);
    while (!file_in.eof())
    {
        std::string next_token;
        std::getline(file_in, next_token);
        if (next_token == "output_file_name")
        {
            file_in >> project->output_file_name;
        }
        else if (next_token == "camera_info")
        {
            auto new_camera_info = std::make_shared<CameraInfo>();
            loadCameraInfo(new_camera_info, file_in);
            project->build_info->cameras_info.push_back(new_camera_info);
        }
        else if (next_token == "aux_geometry_box")
        {
            auto new_aux_geometry_box = std::make_shared<AuxGeometryBox>();
            loadAuxGeometryBox(new_aux_geometry_box, file_in);
            project->aux_geometry->boxes.push_back(new_aux_geometry_box);
        }
        else if (next_token == "vertex")
        {
            auto new_vertex = std::make_shared<Vertex>();
            loadVertex(new_vertex, file_in);
            project->build_info->vertices.push_back(new_vertex);
        }
    }
    linkProject(project);
}

inline void saveMeshProject(const MeshProject::Ptr& project, const std::string& file_name)
{
    std::ofstream file_out(file_name);
    file_out << "# Gkm-World Mesh Builder project file\n";
    file_out << "output_file_name\n" << project->output_file_name << "\n";
    for (auto camera_info : project->build_info->cameras_info)
    {
        saveCameraInfo(camera_info, file_out);
    }
    for (auto box : project->aux_geometry->boxes)
    {
        saveAuxGeometryBox(box, file_out);
    }
    for (auto vertex : project->build_info->vertices)
    {
        saveVertex(vertex, file_out);
    }
}
