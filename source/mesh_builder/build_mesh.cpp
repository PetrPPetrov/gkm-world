// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include <cstdint>
#include <cmath>
#include <vector>
#include <boost/filesystem.hpp>
#include <QRgb>
#include "mesh.h"
#include "build_mesh.h"
#include "bilinear_interpolation.h"
#include "texture_atlas.h"
#include "global_parameters.h"
#include "task.h"

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

void calculateVertexLinePoints(
    const Camera::Ptr& camera, int x, int y,
    Eigen::Vector3d& start, Eigen::Vector3d& finish)
{
    const int camera_width = cameraGetWidth(camera);
    const int camera_height = cameraGetHeight(camera);
    const double camera_aspect = static_cast<double>(camera_width) / camera_height;
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

static void saveMeshObj(const std::string& filename, const Mesh::Ptr& mesh, const std::string& mesh_project_filename, const std::string& mtl_filename)
{
    Job job(3, "Saving to .OBJ...");
    std::ofstream file_out(filename);
    file_out << "# Gkm-World Mesh Builder OBJ File : '" << mesh_project_filename << "'\n";
    file_out << "# http://gkmsoft.xyz\n";
    file_out << "mtllib " << mtl_filename << "\n";
    file_out << "usemtl Textured\n";

    {
        Job job(mesh->vertices.size(), "Saving vertices...");
        for (auto& vertex : mesh->vertices)
        {
            file_out << "v " << vertex.x() << " " << vertex.y() << " " << vertex.z() << "\n";
            job.step();
        }
    }
    {
        Job job(mesh->triangle_tex_coords.size(), "Saving tex coordinates...");
        for (auto& tex_coord : mesh->triangle_tex_coords)
        {
            file_out << "vt " << tex_coord.x() << " " << tex_coord.y() << "\n";
            job.step();
        }
    }
    {
        unsigned tex_coord_index = 1;
        Job job(mesh->triangles.size(), "Saving triangles...");
        for (auto& triangle : mesh->triangles)
        {
            file_out << "f "
                << triangle.x() + 1 << "/" << tex_coord_index << " "
                << triangle.y() + 1 << "/" << tex_coord_index + 1 << " "
                << triangle.z() + 1 << "/" << tex_coord_index + 2 << "\n";
            tex_coord_index += 3;
            job.step();
        }
    }
}

static void saveMeshMtl(const std::string& filename, const std::string& texture_atlas_filename, const std::string& mesh_project_filename)
{
    std::ofstream file_out(filename);
    file_out << "# Gkm-World Mesh Builder MTL File : '" << mesh_project_filename << "'\n";
    file_out << "# http://gkmsoft.xyz\n";
    file_out << "newmtl Textured\n";
    file_out << "map_Kd " << texture_atlas_filename << "\n";
}

static void calculateVertices(const MeshProject::Ptr& mesh_project, const Mesh::Ptr& new_mesh)
{
    const size_t source_vertex_count = mesh_project->vertices.size();

    Job job(source_vertex_count, "Calculating vertex coordinates...");
    new_mesh->vertices.reserve(source_vertex_count);
    new_mesh->old_to_new_vertex_id_map.resize(source_vertex_count, source_vertex_count);
    new_mesh->new_to_old_vertex_id_map.resize(source_vertex_count, source_vertex_count);
    for (size_t i = 0; i < source_vertex_count; ++i)
    {
        const auto& vertex = mesh_project->vertices[i];
        if (vertex->id == -1)
        {
            goto next_iteration; // Yes, I use goto statements!
        }

        if (vertex->positions.size() >= 2)
        {
            Eigen::Vector3d p1;
            Eigen::Vector3d p2;
            calculateVertexLinePoints(
                projectGetCamera(mesh_project, vertex->positions[0]->camera_id),
                vertex->positions[0]->x, vertex->positions[0]->y, p1, p2
            );
            Eigen::Vector3d p3;
            Eigen::Vector3d p4;
            calculateVertexLinePoints(
                projectGetCamera(mesh_project, vertex->positions[1]->camera_id),
                vertex->positions[1]->x, vertex->positions[1]->y, p3, p4
            );
            Eigen::Vector3d pa;
            Eigen::Vector3d pb;
            double mua;
            double mub;
            if (calculateLineLineIntersect(p1, p2, p3, p4, pa, pb, mua, mub))
            {
                Eigen::Vector3d vertex_point = (pa + pb) / 2.0;
                new_mesh->old_to_new_vertex_id_map[i] = new_mesh->vertices.size();
                new_mesh->new_to_old_vertex_id_map[new_mesh->vertices.size()] = i;
                new_mesh->vertices.push_back(vertex_point);
            }
        }

next_iteration:
        job.step();
    }
}

static void mapTriangles(const MeshProject::Ptr& mesh_project, const Mesh::Ptr& new_mesh)
{
    const size_t source_triangle_count = mesh_project->triangles.size();

    Job job(source_triangle_count, "Generating result triangles...");
    const size_t source_vertex_count = mesh_project->vertices.size();
    new_mesh->triangles.reserve(source_triangle_count);
    //new_mesh->old_to_new_vertex_id_map.resize(source_triangle_count, source_triangle_count);
    for (size_t i = 0; i < source_triangle_count; ++i)
    {
        const Triangle& triangle = *mesh_project->triangles[i];
        bool all_vertices_are_ok = true;
        Vector3u new_vertex_indices;

        for (unsigned char j = 0; j < Triangle::vertex_count; ++j)
        {
            bool vertex_ok = false;
            const int old_vertex_id = triangle.vertices[j];
            const size_t old_vertex_id_size_t = static_cast<size_t>(old_vertex_id);
            size_t new_vertex_id;
            if (old_vertex_id >= 0 && old_vertex_id_size_t < new_mesh->old_to_new_vertex_id_map.size())
            {
                new_vertex_id = new_mesh->old_to_new_vertex_id_map[old_vertex_id_size_t];
                if (new_vertex_id >= source_vertex_count)
                {
                    all_vertices_are_ok = false;
                    break;
                }
                new_vertex_indices[j] = static_cast<std::uint32_t>(new_vertex_id);
            }
        }

        if (all_vertices_are_ok)
        {
            //new_mesh->old_to_new_triangle_id_map[i] = new_mesh->triangles.size();
            new_mesh->triangles.push_back(new_vertex_indices);
        }

        job.step();
    }
}

static void buildTexture(const MeshProject::Ptr& mesh_project, const Mesh::Ptr& new_mesh)
{
    TextureAtlas::Ptr texture_atlas = std::make_shared<TextureAtlas>(mesh_project, new_mesh);
    const size_t new_triangle_count = new_mesh->triangles.size();

    Job job(2, "Building triangle textures...");

    double density = 0.0;
    {
        Job job(new_triangle_count, "Calculating triangle texture density...");

        size_t valid_triangle_count = 0;
        for (size_t i = 0; i < new_triangle_count; ++i)
        {
            TriangleTexture::Ptr triangle_texture = std::make_shared<TriangleTexture>(mesh_project, new_mesh, i);
            texture_atlas->triangle_textures.push_back(triangle_texture);

            job.step();

            if (!triangle_texture->isValid())
            {
                continue;
            }

            ++valid_triangle_count;
            if (mesh_project->atlas_density_mode == EDensityMode::Maximum)
            {
                density = std::max(density, triangle_texture->getDensity());
            }
            else
            {
                density += triangle_texture->getDensity();
            }
        }

        if (mesh_project->atlas_density_mode != EDensityMode::Maximum && valid_triangle_count > 0)
        {
            density /= valid_triangle_count;
            postProcessDensityMode(density, mesh_project->atlas_density_mode);
        }

        if (density == 0.0)
        {
            density = 1.0;
        }
    }

    {
        Job job(new_triangle_count, "Preparing triangle textures...");
        for (size_t i = 0; i < new_triangle_count; ++i)
        {
            texture_atlas->triangle_textures[i]->prepareTexture(density);
            job.step();
        }
    }

    texture_atlas->build();
}

void buildMesh(const MeshProject::Ptr& mesh_project)
{
    Job job(5, "Building resulting mesh...");


    // TODO: Add other texture modes support
    Mesh::Ptr new_mesh = std::make_shared<Mesh>();

    calculateVertices(mesh_project, new_mesh);
    mapTriangles(mesh_project, new_mesh);
    buildTexture(mesh_project, new_mesh);

    boost::filesystem::path output_obj_path(mesh_project->output_file_name);
    boost::filesystem::path output_mtl_path = output_obj_path;
    output_mtl_path.replace_extension(".mtl");
    boost::filesystem::path texture_atlas_path = output_obj_path;
    texture_atlas_path.replace_extension(".jpg");

    saveMeshObj(mesh_project->output_file_name, new_mesh, mesh_project->file_name, output_mtl_path.filename().generic_string());
    saveMeshMtl(output_mtl_path.generic_string(), texture_atlas_path.filename().generic_string(), mesh_project->file_name);

    Job save_jpeg_job(1, "Saving texture atlas...");
    new_mesh->texture_atlas->saveJpeg(texture_atlas_path.generic_string().c_str());
    save_jpeg_job.step();
}
