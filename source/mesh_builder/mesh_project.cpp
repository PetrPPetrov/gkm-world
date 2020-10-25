// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include <fstream>
#include <unordered_set>
#include "mesh_project.h"

int cameraGetWidth(const Camera::Ptr& camera)
{
    if (camera->rotation == 0 || camera->rotation == 180)
    {
        return camera->photo_image->width();
    }
    else
    {
        return camera->photo_image->height();
    }
}

int cameraGetHeight(const Camera::Ptr& camera)
{
    if (camera->rotation == 0 || camera->rotation == 180)
    {
        return camera->photo_image->height();
    }
    else
    {
        return camera->photo_image->width();
    }
}

int cameraGetRotationIndex(const Camera::Ptr& camera)
{
    switch (camera->rotation)
    {
    default:
    case 0:
        return 0;
    case 90:
        return 1;
    case 180:
        return 2;
    case 270:
        return 3;
    }
}

void cameraSetRotationFromIndex(const Camera::Ptr& camera, int rotation_index)
{
    int new_rotation = 0;
    switch (rotation_index)
    {
    default:
    case 0:
        new_rotation = 0;
        break;
    case 1:
        new_rotation = 90;
        break;
    case 2:
        new_rotation = 180;
        break;
    case 3:
        new_rotation = 270;
        break;
    }
    camera->rotation = new_rotation;
}

static std::string loadToken(const std::string& expected_line, std::ifstream& file_in)
{
    std::string read_line;
    while (!file_in.eof())
    {
        std::getline(file_in, read_line);
        if (expected_line == read_line)
        {
            return read_line;
        }
    }
    return read_line;
}

static void loadEigenVector3d(Eigen::Vector3d& vector, std::ifstream& file_in)
{
    loadToken("x", file_in);
    file_in >> vector.x();
    loadToken("y", file_in);
    file_in >> vector.y();
    loadToken("z", file_in);
    file_in >> vector.z();
}

static void saveEigenVector3d(const Eigen::Vector3d& vector, std::ofstream& file_out)
{
    file_out << "x\n";
    file_out << vector.x() << "\n";
    file_out << "y\n";
    file_out << vector.y() << "\n";
    file_out << "z\n";
    file_out << vector.z() << "\n";
}

static void loadString(std::string& value, std::ifstream& file_in)
{
    file_in >> value;
    if (value == "null")
    {
        value = "";
    }
}

static void saveString(std::string& value, std::ofstream& file_out)
{
    file_out << (value.empty() ? "null" : value) << "\n";
}

static AuxBox::Ptr loadAuxBox(std::ifstream& file_in)
{
    auto box = std::make_shared<AuxBox>();
    loadToken("id", file_in);
    file_in >> box->id;
    loadToken("position", file_in);
    loadEigenVector3d(box->position, file_in);
    loadToken("size", file_in);
    loadEigenVector3d(box->size, file_in);
    return box;
}

static void saveAuxBox(const AuxBox::Ptr& box, std::ofstream& file_out)
{
    file_out << "aux_box\n";
    file_out << "id\n" << box->id << "\n";
    file_out << "position\n";
    saveEigenVector3d(box->position, file_out);
    file_out << "size\n";
    saveEigenVector3d(box->size, file_out);
}

static Camera::Ptr loadCamera(std::ifstream& file_in)
{
    auto camera = std::make_shared<Camera>();
    loadToken("id", file_in);
    file_in >> camera->id;
    loadToken("photo_image_path", file_in);
    loadString(camera->photo_image_path, file_in);
    loadToken("viewer_pos", file_in);
    loadEigenVector3d(camera->viewer_pos, file_in);
    loadToken("viewer_target", file_in);
    loadEigenVector3d(camera->viewer_target, file_in);
    loadToken("viewer_up", file_in);
    loadEigenVector3d(camera->viewer_up, file_in);
    loadToken("rotation_radius", file_in);
    file_in >> camera->rotation_radius;
    loadToken("locked", file_in);
    file_in >> camera->locked;
    loadToken("rotation", file_in);
    file_in >> camera->rotation;
    loadToken("fov", file_in);
    file_in >> camera->fov;
    return camera;
}

static void saveCamera(const Camera::Ptr& camera, std::ofstream& file_out)
{
    file_out << "camera\n";
    file_out << "id\n" << camera->id << "\n";
    file_out << "photo_image_path\n";
    saveString(camera->photo_image_path, file_out);
    file_out << "viewer_pos\n";
    saveEigenVector3d(camera->viewer_pos, file_out);
    file_out << "viewer_target\n";
    saveEigenVector3d(camera->viewer_target, file_out);
    file_out << "viewer_up\n";
    saveEigenVector3d(camera->viewer_up, file_out);
    file_out << "rotation_radius\n" << camera->rotation_radius << "\n";
    file_out << "locked\n" << camera->locked << "\n";
    file_out << "rotation\n" << camera->rotation << "\n";
    file_out << "fov\n" << camera->fov << "\n";
}

static Vertex::Ptr loadVertex(std::ifstream& file_in)
{
    auto vertex = std::make_shared<Vertex>();
    loadToken("id", file_in);
    file_in >> vertex->id;
    loadToken("vertex_position_count", file_in);
    size_t count = 0;
    file_in >> count;
    vertex->positions.reserve(count);
    for (size_t i = 0; i < count; ++i)
    {
        auto new_info = std::make_shared<VertexPhotoPosition>();
        loadToken("camera_id", file_in);
        file_in >> new_info->camera_id;
        loadToken("vertex_position_x", file_in);
        file_in >> new_info->x;
        loadToken("vertex_position_y", file_in);
        file_in >> new_info->y;
        new_info->vertex_id = vertex->id;
        vertex->positions.push_back(new_info);
    }
    return vertex;
}

static void saveVertex(const Vertex::Ptr& vertex, std::ofstream& file_out)
{
    file_out << "vertex\n";
    file_out << "id\n" << vertex->id << "\n";
    file_out << "vertex_position_count\n" << vertex->positions.size() << "\n";
    for (auto vertex_position_info : vertex->positions)
    {
        file_out << "camera_id\n" << vertex_position_info->camera_id << "\n";
        file_out << "vertex_position_x\n" << vertex_position_info->x << "\n";
        file_out << "vertex_position_y\n" << vertex_position_info->y << "\n";
    }
}

Triangle::Ptr loadTriangle(std::ifstream& file_in)
{
    auto triangle = std::make_shared<Triangle>();
    loadToken("id", file_in);
    file_in >> triangle->id;
    loadToken("vertex0", file_in);
    file_in >> triangle->vertices[0];
    loadToken("vertex1", file_in);
    file_in >> triangle->vertices[1];
    loadToken("vertex2", file_in);
    file_in >> triangle->vertices[2];
    return triangle;
}

static void saveTriangle(const Triangle::Ptr& triangle, std::ofstream& file_out)
{
    file_out << "triangle\n";
    file_out << "id\n" << triangle->id << "\n";
    file_out << "vertex0\n" << triangle->vertices[0] << "\n";
    file_out << "vertex1\n" << triangle->vertices[1] << "\n";
    file_out << "vertex2\n" << triangle->vertices[2] << "\n";
}

static void linkProject(const MeshProject::Ptr& project)
{
    for (auto& vertex : project->vertices)
    {
        for (auto& vertex_position : vertex->positions)
        {
            for (auto& camera : project->cameras)
            {
                if (vertex_position->camera_id == camera->id)
                {
                    camera->positions.push_back(vertex_position);
                }
            }
        }
    }
}

MeshProject::Ptr loadMeshProject(const std::string& file_name)
{
    auto project = std::make_shared<MeshProject>();
    std::ifstream file_in(file_name);
    loadToken("# Gkm-World Mesh Builder project file", file_in);
    while (!file_in.eof())
    {
        std::string next_token;
        std::getline(file_in, next_token);
        if (next_token == "output_file_name")
        {
            loadString(project->output_file_name, file_in);
        }
        else if (next_token == "camera")
        {
            project->cameras.push_back(loadCamera(file_in));
        }
        else if (next_token == "aux_box")
        {
            project->aux_geometry->boxes.push_back(loadAuxBox(file_in));
        }
        else if (next_token == "vertex")
        {
            project->vertices.push_back(loadVertex(file_in));
        }
        else if (next_token == "triangle")
        {
            project->triangles.push_back(loadTriangle(file_in));
        }
    }
    linkProject(project);
    return project;
}

void saveMeshProject(const MeshProject::Ptr& project, const std::string& file_name)
{
    std::ofstream file_out(file_name);
    file_out << "# Gkm-World Mesh Builder project file\n";
    file_out << "output_file_name\n";
    saveString(project->output_file_name, file_out);
    for (auto& camera : project->cameras)
    {
        saveCamera(camera, file_out);
    }
    for (auto& box : project->aux_geometry->boxes)
    {
        saveAuxBox(box, file_out);
    }
    for (auto& vertex : project->vertices)
    {
        saveVertex(vertex, file_out);
    }
    for (auto& triangle : project->triangles)
    {
        saveTriangle(triangle, file_out);
    }
}

Camera::Ptr projectGetCamera(const MeshProject::Ptr& project, int camera_id)
{
    for (auto& camera : project->cameras)
    {
        if (camera->id == camera_id)
        {
            return camera;
        }
    }
    return nullptr;
}

Camera::Ptr projectAddPhoto(const MeshProject::Ptr& project, const char* filename)
{
    std::unordered_set<int> used_id;
    for (auto& camera : project->cameras)
    {
        used_id.emplace(camera->id);
    }
    int new_id = 0;
    while (used_id.find(new_id) != used_id.end())
    {
        ++new_id;
    }
    auto new_camera = std::make_shared<Camera>();
    new_camera->photo_image_path = filename;
    new_camera->viewer_pos = Eigen::Vector3d(3, 3, 3);
    new_camera->viewer_target = Eigen::Vector3d(0, 0, 0);
    new_camera->viewer_up = Eigen::Vector3d(0, 0, 1);
    new_camera->rotation_radius = (new_camera->viewer_pos - new_camera->viewer_target).norm();
    new_camera->id = new_id;
    project->cameras.push_back(new_camera);
    return new_camera;
}

void projectRemovePhoto(const MeshProject::Ptr& project, const Camera::Ptr& camera)
{
    auto cur_camera_it = project->cameras.begin();
    while (cur_camera_it != project->cameras.end())
    {
        if (*cur_camera_it == camera)
        {
            project->cameras.erase(cur_camera_it);
            break;
        }
        else
        {
            ++cur_camera_it;
        }
    }
    for (auto vertex : project->vertices)
    {
        auto vertex_position_it = vertex->positions.begin();
        while (vertex_position_it != vertex->positions.end())
        {
            if ((*vertex_position_it)->camera_id == camera->id)
            {
                vertex->positions.erase(vertex_position_it);
                vertex_position_it = vertex->positions.begin();
            }
            else
            {
                ++vertex_position_it;
            }
        }
    }
}

AuxBox::Ptr projectAddAuxBox(const MeshProject::Ptr& project)
{
    auto& boxes = project->aux_geometry->boxes;
    const unsigned count = static_cast<unsigned>(boxes.size());
    AuxBox::Ptr new_box;
    for (unsigned i = 0; i < count; ++i)
    {
        if (boxes[i]->id == -1)
        {
            new_box = boxes[i];
            new_box->id = static_cast<int>(i);
            break;
        }
    }
    if (!new_box)
    {
        new_box = std::make_shared<AuxBox>();
        new_box->id = static_cast<int>(count);
        boxes.push_back(new_box);
    }

    new_box->position = Eigen::Vector3d(0, 0, 0);
    new_box->size = Eigen::Vector3d(1, 1, 1);
    return new_box;
}

void projectRemoveAuxBox(const MeshProject::Ptr& project, const AuxBox::Ptr& aux_box)
{
    aux_box->id = -1;
}

Vertex::Ptr projectAddVertex(const MeshProject::Ptr& project)
{
    auto& vertices = project->vertices;
    const unsigned count = static_cast<unsigned>(vertices.size());
    Vertex::Ptr new_vertex;
    for (unsigned i = 0; i < count; ++i)
    {
        if (vertices[i]->id == -1)
        {
            new_vertex = vertices[i];
            vertices[i]->id = static_cast<int>(i);
            vertices[i]->positions.clear();
            break;
        }
    }
    if (!new_vertex)
    {
        new_vertex = std::make_shared<Vertex>();
        new_vertex->id = static_cast<int>(count);
        vertices.push_back(new_vertex);
    }
    return new_vertex;
}

void projectRemoveVertex(const MeshProject::Ptr& project, const Vertex::Ptr& vertex)
{
    vertex->id = -1;
}

VertexPhotoPosition::Ptr projectAddCurrentVertex(const MeshProject::Ptr& project, const Camera::Ptr& camera, const Vertex::Ptr& vertex)
{
    auto new_position_info = std::make_shared<VertexPhotoPosition>();
    new_position_info->vertex_id = vertex->id;
    new_position_info->camera_id = camera->id;
    new_position_info->x = cameraGetWidth(camera) / 2;
    new_position_info->y = cameraGetHeight(camera) / 2;
    camera->positions.push_back(new_position_info);
    vertex->positions.push_back(new_position_info);
    return new_position_info;
}

void projectRemoveCurrentVertex(const MeshProject::Ptr& project, const Camera::Ptr& camera, const Vertex::Ptr& vertex)
{
    auto vertex_position_it = camera->positions.begin();
    while (vertex_position_it != camera->positions.end())
    {
        if ((*vertex_position_it)->vertex_id == vertex->id)
        {
            camera->positions.erase(vertex_position_it);
            vertex_position_it = camera->positions.begin();
        }
        else
        {
            ++vertex_position_it;
        }
    }
    vertex_position_it = vertex->positions.begin();
    while (vertex_position_it != vertex->positions.end())
    {
        if ((*vertex_position_it)->camera_id == camera->id)
        {
            vertex->positions.erase(vertex_position_it);
            vertex_position_it = vertex->positions.begin();
        }
        else
        {
            ++vertex_position_it;
        }
    }
}

Triangle::Ptr projectAddTriangle(const MeshProject::Ptr& project)
{
    auto& triangles = project->triangles;
    const unsigned count = static_cast<unsigned>(triangles.size());
    Triangle::Ptr new_triangle;
    for (unsigned i = 0; i < count; ++i)
    {
        if (triangles[i]->id == -1)
        {
            // Found free vertex
            triangles[i]->id = i;
            triangles[i]->vertices[0] = -1;
            triangles[i]->vertices[1] = -1;
            triangles[i]->vertices[2] = -1;
            new_triangle = triangles[i];
            break;
        }
    }
    if (!new_triangle)
    {
        new_triangle = std::make_shared<Triangle>();
        new_triangle->id = count;
        triangles.push_back(new_triangle);
    }
    return new_triangle;
}

void projectRemoveTriangle(const MeshProject::Ptr& project, const Triangle::Ptr& triangle)
{
    triangle->id = -1;
}

void projectUseVertex(const MeshProject::Ptr& project, const Triangle::Ptr& triangle, int triangle_item, const Vertex::Ptr& vertex)
{
    triangle->vertices[triangle_item] = vertex->id;
}
