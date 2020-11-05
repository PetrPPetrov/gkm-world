// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include "vbo_fill.h"
#include "color_hasher.h"
#include "build_mesh.h"

constexpr static VertexPositionColor g_box_vbo[] =
{
    { 0.0f, 0.0f, 0.0f, 0xff0000ff },
    { 1.0f, 0.0f, 0.0f, 0xff0000ff },
    { 0.0f, 0.0f, 0.0f, 0xff00ff00 },
    { 0.0f, 1.0f, 0.0f, 0xff00ff00 },
    { 0.0f, 0.0f, 0.0f, 0xffff0000 },
    { 0.0f, 0.0f, 1.0f, 0xffff0000 },
    { 1.0f, 0.0f, 0.0f, 0xffffffff },
    { 1.0f, 1.0f, 0.0f, 0xffffffff },
    { 1.0f, 1.0f, 0.0f, 0xffffffff },
    { 0.0f, 1.0f, 0.0f, 0xffffffff },
    { 0.0f, 0.0f, 1.0f, 0xffffffff },
    { 1.0f, 0.0f, 1.0f, 0xffffffff },
    { 0.0f, 0.0f, 1.0f, 0xffffffff },
    { 0.0f, 1.0f, 1.0f, 0xffffffff },
    { 1.0f, 0.0f, 1.0f, 0xffffffff },
    { 1.0f, 1.0f, 1.0f, 0xffffffff },
    { 1.0f, 1.0f, 1.0f, 0xffffffff },
    { 0.0f, 1.0f, 1.0f, 0xffffffff },
    { 1.0f, 0.0f, 0.0f, 0xffffffff },
    { 1.0f, 0.0f, 1.0f, 0xffffffff },
    { 0.0f, 1.0f, 0.0f, 0xffffffff },
    { 0.0f, 1.0f, 1.0f, 0xffffffff },
    { 1.0f, 1.0f, 0.0f, 0xffffffff },
    { 1.0f, 1.0f, 1.0f, 0xffffffff }
};
constexpr size_t g_box_vbo_size = sizeof(g_box_vbo) / sizeof(g_box_vbo[0]);


static void fillAuxGeometryBox(
    std::vector<VertexPositionColor>& vbo,
    const AuxBox::Ptr& box)
{
    for (size_t i = 0; i < g_box_vbo_size; ++i)
    {
        VertexPositionColor cur_vertex = g_box_vbo[i];
        cur_vertex.x = box->position.x() + cur_vertex.x * box->size.x();
        cur_vertex.y = box->position.y() + cur_vertex.y * box->size.y();
        cur_vertex.z = box->position.z() + cur_vertex.z * box->size.z();
        vbo.push_back(cur_vertex);
    }
}

static void fillAuxGeometryBoxes(
    std::vector<VertexPositionColor>& vbo,
    std::vector<AuxBox::Ptr>& boxes)
{
    vbo.reserve(vbo.size() + boxes.size() * g_box_vbo_size);
    for (auto& box : boxes)
    {
        if (box->id != -1)
        {
            fillAuxGeometryBox(vbo, box);
        }
    }
}

static void fillAuxGeometry(
    std::vector<VertexPositionColor>& vbo,
    const AuxGeometry::Ptr& aux_geometry,
    int& aux_geom_line_set_vbo_start,
    int& aux_geom_line_set_vbo_size)
{
    aux_geom_line_set_vbo_start = static_cast<int>(vbo.size());
    if (aux_geometry)
    {
        fillAuxGeometryBoxes(vbo, aux_geometry->boxes);
    }

    aux_geom_line_set_vbo_size = static_cast<int>(vbo.size()) - aux_geom_line_set_vbo_start;
}

constexpr static VertexPositionColor g_hud_point_vbo[] =
{
    { -30.0f,   0.0f, 0.0f, 0xff0000ff },
    {  30.0f,   0.0f, 0.0f, 0xff0000ff },
    {   0.0f, -30.0f, 0.0f, 0xff0000ff },
    {   0.0f,  30.0f, 0.0f, 0xff0000ff }
};
constexpr size_t g_hud_point_vbo_size = sizeof(g_hud_point_vbo) / sizeof(g_hud_point_vbo[0]);

struct VertexInfo
{
    int id;
    int x = 0;
    int y = 0;
};

static void fillHubPoints(
    std::vector<VertexPositionColor>& vbo,
    const MeshProject::Ptr& mesh_project,
    const Camera::Ptr current_camera,
    int& hub_points_line_set_vbo_start,
    int& hub_points_line_set_vbo_size,
    int photo_x_low, int photo_y_low)
{
    hub_points_line_set_vbo_start = static_cast<int>(vbo.size());

    std::list<VertexInfo> vertices;
    if (current_camera)
    {
        auto& cameras = mesh_project->cameras;
        const unsigned camera_count = static_cast<unsigned>(cameras.size());
        for (auto& vertex : mesh_project->vertices)
        {
            if (vertex->id != -1)
            {
                for (auto& position : vertex->positions)
                {
                    Camera::Ptr used_camera = nullptr;
                    if (position->camera_id == current_camera->id)
                    {
                        vertices.push_back({ vertex->id, position->x, position->y });
                        break;
                    }
                }
            }
        }
        vbo.reserve(vbo.size() + vertices.size() * g_hud_point_vbo_size);
        for (auto& vertex : vertices)
        {
            for (size_t i = 0; i < g_hud_point_vbo_size; ++i)
            {
                VertexPositionColor cur_vertex = g_hud_point_vbo[i];
                cur_vertex.x += photo_x_low + vertex.x;
                cur_vertex.y += photo_y_low + vertex.y; // TODO: Change color
                cur_vertex.z = -1.0f;
                cur_vertex.abgr = ColorHasher::getColor(vertex.id);
                vbo.push_back(cur_vertex);
            }
        }
    }
    hub_points_line_set_vbo_size = static_cast<int>(vbo.size()) - hub_points_line_set_vbo_start;
}

struct VertexProjectionInfo
{
    Camera::Ptr camera = nullptr;
    int id;
    int x = 0;
    int y = 0;
};

static void fillVertices(
    std::vector<VertexPositionColor>& vbo,
    const MeshProject::Ptr& mesh_project,
    const Camera::Ptr current_camera,
    int& vertices_line_set_vbo_start,
    int& vertices_line_set_vbo_size,
    int photo_x_low, int photo_y_low)
{
    vertices_line_set_vbo_start = static_cast<int>(vbo.size());

    std::list<VertexProjectionInfo> vertices;
    for (auto& vertex : mesh_project->vertices)
    {
        if (vertex->id != -1)
        {
            for (auto& position : vertex->positions)
            {
                Camera::Ptr used_camera = projectGetCamera(mesh_project, position->camera_id);
                if (used_camera)
                {
                    vertices.push_back({ used_camera, vertex->id, position->x, position->y });
                }
            }
        }
    }
    vbo.reserve(vbo.size() + vertices.size() * 2);
    for (auto& vertex : vertices)
    {
        Eigen::Vector3d start;
        Eigen::Vector3d finish;
        calculateVertexLinePoints(vertex.camera, vertex.x, vertex.y, start, finish);

        VertexPositionColor first_vertex;
        first_vertex.x = static_cast<float>(start.x());
        first_vertex.y = static_cast<float>(start.y());
        first_vertex.z = static_cast<float>(start.z());
        first_vertex.abgr = ColorHasher::getColor(vertex.id);
        vbo.push_back(first_vertex);

        VertexPositionColor second_vertex;
        second_vertex.x = static_cast<float>(finish.x());
        second_vertex.y = static_cast<float>(finish.y());
        second_vertex.z = static_cast<float>(finish.z());
        second_vertex.abgr = ColorHasher::getColor(vertex.id);
        vbo.push_back(second_vertex);
    }

    vertices_line_set_vbo_size = static_cast<int>(vbo.size()) - vertices_line_set_vbo_start;
}

static void correctOffsets(int& start, int& size)
{
    if (start >= LINE_SET_VBO_MAX_VERTEX_COUNT)
    {
        start = LINE_SET_VBO_MAX_VERTEX_COUNT;
        size = 0;
    }
    else if (start + size >= LINE_SET_VBO_MAX_VERTEX_COUNT)
    {
        size = LINE_SET_VBO_MAX_VERTEX_COUNT - start;
    }
}

void fillLineSet(
    std::vector<VertexPositionColor>& vbo,
    const MeshProject::Ptr& mesh_project,
    const Camera::Ptr current_camera,
    int& aux_geom_line_set_vbo_start,
    int& aux_geom_line_set_vbo_size,
    int& hub_points_line_set_vbo_start,
    int& hub_points_line_set_vbo_size,
    int& vertices_line_set_vbo_start,
    int& vertices_line_set_vbo_size,
    int photo_x_low, int photo_y_low)
{
    vbo.clear();

    if (!mesh_project)
    {
        aux_geom_line_set_vbo_start = aux_geom_line_set_vbo_size = 0;
        vertices_line_set_vbo_start = vertices_line_set_vbo_size = 0;
        hub_points_line_set_vbo_start = hub_points_line_set_vbo_size = 0;
        return;
    }

    fillAuxGeometry(vbo, mesh_project->aux_geometry, aux_geom_line_set_vbo_start, aux_geom_line_set_vbo_size);
    fillVertices(vbo, mesh_project, current_camera, vertices_line_set_vbo_start, vertices_line_set_vbo_size, photo_x_low, photo_y_low);
    fillHubPoints(vbo, mesh_project, current_camera, hub_points_line_set_vbo_start, hub_points_line_set_vbo_size, photo_x_low, photo_y_low);

    correctOffsets(aux_geom_line_set_vbo_start, aux_geom_line_set_vbo_size);
    correctOffsets(vertices_line_set_vbo_start, vertices_line_set_vbo_size);
    correctOffsets(hub_points_line_set_vbo_start, hub_points_line_set_vbo_size);
}
