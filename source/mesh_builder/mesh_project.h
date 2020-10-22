// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <memory>
#include <string>
#include <cstdint>
#include <vector>
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

    int id = -1;
    int vertices[3] = { -1, -1, -1 };
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

struct MeshProject
{
    typedef std::shared_ptr<MeshProject> Ptr;

    AuxGeometry::Ptr aux_geometry = std::make_shared<AuxGeometry>();

    std::vector<Camera::Ptr> cameras;
    std::vector<Vertex::Ptr> vertices;
    std::vector<Triangle::Ptr> triangles;

    std::string file_name;
    std::string output_file_name;
    bool dirty = false;
};

MeshProject::Ptr loadMeshProject(const std::string& file_name);
void saveMeshProject(const MeshProject::Ptr& project, const std::string& file_name);

void projectAddPhoto(const MeshProject::Ptr& project, const char* filename);
void projectRemovePhoto(const MeshProject::Ptr& project, const Camera::Ptr& camera);

void projectAddAuxBox(const MeshProject::Ptr& project);
void projectRemoveAuxBox(const MeshProject::Ptr& project, const AuxBox::Ptr& aux_box);
