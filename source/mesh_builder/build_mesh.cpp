// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include <vector>
#include "build_mesh.h"
#include "global_parameters.h"

constexpr static double EPSILON = 0.001;

// Adopted from http://paulbourke.net/geometry/pointlineplane/lineline.c
// Calculate the line segment PaPb that is the shortest route between
// two lines P1P2 and P3P4. Calculate also the values of mua and mub where
//    Pa = P1 + mua (P2 - P1)
//    Pb = P3 + mub (P4 - P3)
// Return false if no solution exists.
static inline bool calculateLineLineIntersect(
    const Eigen::Vector3d& p1, const Eigen::Vector3d& p2,
    const Eigen::Vector3d& p3, const Eigen::Vector3d& p4,
    Eigen::Vector3d& pa, Eigen::Vector3d& pb,
    double& mua, double& mub)
{
    Eigen::Vector3d p13, p43, p21;

    p13.x() = p1.x() - p3.x();
    p13.y() = p1.y() - p3.y();
    p13.z() = p1.z() - p3.z();
    p43.x() = p4.x() - p3.x();
    p43.y() = p4.y() - p3.y();
    p43.z() = p4.z() - p3.z();
    if (fabs(p43.x()) < EPSILON && fabs(p43.y()) < EPSILON && fabs(p43.z()) < EPSILON)
    {
        return false;
    }
    p21.x() = p2.x() - p1.x();
    p21.y() = p2.y() - p1.y();
    p21.z() = p2.z() - p1.z();
    if (fabs(p21.x()) < EPSILON && fabs(p21.y()) < EPSILON && fabs(p21.z()) < EPSILON)
    {
        return false;
    }

    const double d1343 = p13.dot(p43);
    const double d4321 = p43.dot(p21);
    const double d1321 = p13.dot(p21);
    const double d4343 = p43.squaredNorm();
    const double d2121 = p21.squaredNorm();

    const double denom = d2121 * d4343 - d4321 * d4321;
    if (fabs(denom) < EPSILON)
    {
        return false;
    }
    const double numer = d1343 * d4321 - d1321 * d4343;

    mua = numer / denom;
    mub = (d1343 + d4321 * mua) / d4343;

    pa.x() = p1.x() + mua * p21.x();
    pa.y() = p1.y() + mua * p21.y();
    pa.z() = p1.z() + mua * p21.z();
    pb.x() = p3.x() + mub * p43.x();
    pb.y() = p3.y() + mub * p43.y();
    pb.z() = p3.z() + mub * p43.z();

    return true;
}

Camera::Ptr getCamera(const MeshProject::Ptr& mesh_project, int camera_index)
{
    auto& cameras = mesh_project->cameras;
    const int camera_count = static_cast<int>(cameras.size());
    Camera::Ptr used_camera = nullptr;
    if (camera_index >= 0 && camera_index < camera_count)
    {
        used_camera = cameras[camera_index];
    }
    return used_camera;
}

void calculateVertexLinePoints(
    const Camera::Ptr& camera, int x, int y,
    Eigen::Vector3d& start, Eigen::Vector3d& finish)
{
    const int camera_width = cameraGetWidth(camera);
    const int camera_height = cameraGetHeight(camera);
    const double camera_aspect = static_cast<double>(camera_width / camera_height);
    const double forward_offset = 16.0;
    const double full_offset_y = forward_offset * tan(GRAD_TO_RAD * camera->fov / 2.0);
    const double full_offset_x = full_offset_y * camera_aspect;
    Eigen::Vector3d pos = camera->viewer_pos;
    Eigen::Vector3d forward = (camera->viewer_target - pos).normalized() * forward_offset;
    Eigen::Vector3d up = camera->viewer_up.normalized() * full_offset_y;
    Eigen::Vector3d right = forward.cross(up).normalized() * full_offset_x;
    const double rel_x = 2.0 * x / camera_width - 1.0;
    const double rel_y = 2.0 * y / camera_height - 1.0;
    Eigen::Vector3d vertex_direction = forward + right * rel_x + up * rel_y;
    Eigen::Vector3d vertex_dir = vertex_direction.normalized() * 2048.0;
    Eigen::Vector3d end_vertex_line = pos + vertex_dir;

    start = camera->viewer_pos;
    finish = end_vertex_line;
}

struct Mesh
{
    typedef std::shared_ptr<Mesh> Ptr;

    std::vector<Eigen::Vector3d> vertices;
    std::vector<size_t> old_to_new_id_map;
};

static void saveMeshObj(const std::string& filename, const Mesh::Ptr& mesh, const std::string& mesh_project_filename)
{
    std::ofstream file_out(filename);
    file_out << "# Gkm-World Mesh Builder OBJ File : '" << mesh_project_filename << "'\n";
    file_out << "# http://gkmsoft.xyz\n";
    for (auto& vertex : mesh->vertices)
    {
        file_out << "v " << vertex.x() << " " << vertex.y() << " " << vertex.z() << "\n";
    }
}

void buildMesh(const MeshProject::Ptr& mesh_project)
{
    Mesh::Ptr new_mesh = std::make_shared<Mesh>();
    const size_t source_vertex_count = mesh_project->vertices.size();
    new_mesh->vertices.reserve(source_vertex_count);
    new_mesh->old_to_new_id_map.resize(source_vertex_count, source_vertex_count);

    for (size_t i = 0; i < source_vertex_count; ++i)
    {
        const auto& vertex = mesh_project->vertices[i];
        if (vertex->id == -1)
        {
            continue;
        }

        if (vertex->positions.size() >= 2)
        {
            Eigen::Vector3d p1;
            Eigen::Vector3d p2;
            calculateVertexLinePoints(
                getCamera(mesh_project, vertex->positions[0]->camera_id),
                vertex->positions[0]->x, vertex->positions[0]->y, p1, p2
            );
            Eigen::Vector3d p3;
            Eigen::Vector3d p4;
            calculateVertexLinePoints(
                getCamera(mesh_project, vertex->positions[1]->camera_id),
                vertex->positions[1]->x, vertex->positions[1]->y, p3, p4
            );
            Eigen::Vector3d pa;
            Eigen::Vector3d pb;
            double mua;
            double mub;
            if (calculateLineLineIntersect(p1, p2, p3, p4, pa, pb, mua, mub))
            {
                Eigen::Vector3d vertex_point = (pa + pb) / 2.0;
                new_mesh->old_to_new_id_map[i] = new_mesh->vertices.size();
                new_mesh->vertices.push_back(vertex_point);
            }
        }
    }

    saveMeshObj(mesh_project->output_file_name, new_mesh, mesh_project->file_name);
}
