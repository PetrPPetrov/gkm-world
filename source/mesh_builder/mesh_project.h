// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <memory>
#include <string>
#include <cstdint>
#include <vector>
#include <array>
#include "common.h"

struct AuxBox
{
    typedef std::shared_ptr<AuxBox> Ptr;

    int id = -1;
    Eigen::Vector3d position;
    Eigen::Vector3d size;
};

struct AuxGeometry
{
    typedef std::shared_ptr<AuxGeometry> Ptr;

    std::vector<AuxBox::Ptr> boxes;
};

struct VertexPhotoPosition
{
    typedef std::shared_ptr<VertexPhotoPosition> Ptr;

    unsigned camera_id = 0;
    unsigned vertex_id = 0;
    int x = 0;
    int y = 0;
};

struct Vertex
{
    typedef std::shared_ptr<Vertex> Ptr;

    int id = -1;
    std::vector<VertexPhotoPosition::Ptr> positions;
};

struct Triangle
{
    typedef std::shared_ptr<Triangle> Ptr;

    constexpr static unsigned char vertex_count = 3;

    int id = -1;
    std::array<int, vertex_count> vertices = { -1, -1, -1 };
};

struct Camera
{
    typedef std::shared_ptr<Camera> Ptr;

    int id = -1;
    std::string photo_image_path;
    ImagePtr photo_image;
    Eigen::Vector3d viewer_pos;
    Eigen::Vector3d viewer_target;
    Eigen::Vector3d viewer_up;
    double rotation_radius;

    bool locked = false;
    int rotation = 0;
    double fov = 50.0;

    std::vector<VertexPhotoPosition::Ptr> positions;
};

int cameraGetWidth(const Camera::Ptr& camera);
int cameraGetHeight(const Camera::Ptr& camera);
int cameraGetRotationIndex(const Camera::Ptr& camera);
void cameraSetRotationFromIndex(const Camera::Ptr& camera, int rotation_index);

enum class EDensityMode
{
    Average,
    Maximum
};

inline int getIndex(EDensityMode density_mode)
{
    return static_cast<int>(density_mode);
}

inline EDensityMode getDensityMode(int index)
{
    return static_cast<EDensityMode>(index);
}

struct MeshProject
{
    typedef std::shared_ptr<MeshProject> Ptr;

    AuxGeometry::Ptr aux_geometry = std::make_shared<AuxGeometry>();

    std::vector<Camera::Ptr> cameras;
    std::vector<Vertex::Ptr> vertices;
    std::vector<Triangle::Ptr> triangles;

    int protection_offset = 8;
    int scale = 256.0;
    int rotation_count = 8;
    int population_size = 128;
    int generation_count = 32;
    int mutation_rate = 10;
    unsigned max_texture_size = 1024;
    EDensityMode triangle_density_mode = EDensityMode::Average;
    EDensityMode atlas_density_mode = EDensityMode::Average;

    std::string file_name;
    std::string output_file_name;
    bool dirty = false;
};

MeshProject::Ptr loadMeshProject(const std::string& file_name);
void saveMeshProject(const MeshProject::Ptr& project, const std::string& file_name);

Camera::Ptr projectGetCamera(const MeshProject::Ptr& project, int camera_id);

Camera::Ptr projectAddPhoto(const MeshProject::Ptr& project, const char* filename);
void projectRemovePhoto(const MeshProject::Ptr& project, const Camera::Ptr& camera);

AuxBox::Ptr projectAddAuxBox(const MeshProject::Ptr& project);
void projectRemoveAuxBox(const MeshProject::Ptr& project, const AuxBox::Ptr& aux_box);

Vertex::Ptr projectAddVertex(const MeshProject::Ptr& project);
void projectRemoveVertex(const MeshProject::Ptr& project, const Vertex::Ptr& vertex);

VertexPhotoPosition::Ptr projectAddCurrentVertex(const MeshProject::Ptr& project, const Camera::Ptr& camera, const Vertex::Ptr& vertex);
void projectRemoveCurrentVertex(const MeshProject::Ptr& project, const Camera::Ptr& camera, const Vertex::Ptr& vertex);

Triangle::Ptr projectAddTriangle(const MeshProject::Ptr& project);
void projectRemoveTriangle(const MeshProject::Ptr& project, const Triangle::Ptr& triangle);

void projectUseVertex(const MeshProject::Ptr& project, const Triangle::Ptr& triangle, int triangle_item, const Vertex::Ptr& vertex);
