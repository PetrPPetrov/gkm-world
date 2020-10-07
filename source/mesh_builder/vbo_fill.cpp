// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include "vbo_fill.h"

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
    const AuxGeometryBox::Ptr& box)
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
    std::list<AuxGeometryBox::Ptr>& boxes)
{
    vbo.reserve(vbo.size() + boxes.size() * g_box_vbo_size);
    for (auto& box : boxes)
    {
        fillAuxGeometryBox(vbo, box);
    }
}

static void fillAuxGeometry(
    std::vector<VertexPositionColor>& vbo,
    const AuxGeometry::Ptr& aux_geometry,
    int& aux_geom_line_set_vbo_size)
{
    const size_t previous_vbo_size = vbo.size();
    if (aux_geometry)
    {
        fillAuxGeometryBoxes(vbo, aux_geometry->boxes);
    }
    aux_geom_line_set_vbo_size = static_cast<int>(vbo.size() - previous_vbo_size);
}

constexpr static VertexPositionColor g_hud_point_vbo[] =
{
    { -10.0f,   0.0f, 0.0f, 0xff0000ff },
    {  10.0f,   0.0f, 0.0f, 0xff0000ff },
    {   0.0f, -10.0f, 0.0f, 0xff0000ff },
    {   0.0f,  10.0f, 0.0f, 0xff0000ff },
    { -10.0f, -10.0f, 0.0f, 0xff0000ff },
    {  10.0f,  10.0f, 0.0f, 0xff0000ff },
    {  10.0f, -10.0f, 0.0f, 0xff0000ff },
    { -10.0f,  10.0f, 0.0f, 0xff0000ff }
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
    const CameraInfo::Ptr cur_camera,
    int& hub_points_line_set_vbo_size,
    int photo_x_low, int photo_y_low)
{
    const size_t previous_vbo_size = vbo.size();
    if (mesh_project->build_info)
    {
        std::list<VertexInfo> vertices;
        auto& cameras = mesh_project->build_info->cameras_info;
        const int camera_count = static_cast<int>(cameras.size());
        for (auto& vertex : mesh_project->build_info->vertices)
        {
            for (auto& position : vertex->positions)
            {
                CameraInfo::Ptr used_camera = nullptr;
                if (position.camera_index >= 0 && position.camera_index < camera_count)
                {
                    used_camera = cameras[position.camera_index];
                }
                if (used_camera == cur_camera)
                {
                    vertices.push_back({ vertex->id, position.x, position.y });
                    break;
                }
            }
        }
        vbo.reserve(vbo.size() + vertices.size() * g_hud_point_vbo_size);
        for (auto& vertex : vertices)
        {
            for (size_t i = 0; i < g_hud_point_vbo_size; ++i)
            {
                VertexPositionColor cur_vertex = g_hud_point_vbo[i];
                cur_vertex.x += vertex.x - photo_x_low;
                cur_vertex.y += vertex.y - photo_y_low; // TODO: Change color
                cur_vertex.abgr = 0xff0000ff;
                vbo.push_back(cur_vertex);
            }
        }
    }
    hub_points_line_set_vbo_size = static_cast<int>(vbo.size() - previous_vbo_size);
}

void fillLineSet(
    std::vector<VertexPositionColor>& vbo,
    const MeshProject::Ptr& mesh_project,
    const CameraInfo::Ptr cur_camera,
    int& aux_geom_line_set_vbo_size,
    int& hub_points_line_set_vbo_size,
    int photo_x_low, int photo_y_low)
{
    vbo.clear();

    if (!mesh_project)
    {
        aux_geom_line_set_vbo_size = 0;
        hub_points_line_set_vbo_size = 0;
        return;
    }

    fillAuxGeometry(vbo, mesh_project->aux_geometry, aux_geom_line_set_vbo_size);
    fillHubPoints(vbo, mesh_project, cur_camera, hub_points_line_set_vbo_size, photo_x_low, photo_y_low);

    if (aux_geom_line_set_vbo_size + hub_points_line_set_vbo_size > LINE_SET_VBO_MAX_VERTEX_COUNT)
    {
        hub_points_line_set_vbo_size = LINE_SET_VBO_MAX_VERTEX_COUNT - aux_geom_line_set_vbo_size;
    }
    if (aux_geom_line_set_vbo_size > LINE_SET_VBO_MAX_VERTEX_COUNT)
    {
        aux_geom_line_set_vbo_size = LINE_SET_VBO_MAX_VERTEX_COUNT;
        hub_points_line_set_vbo_size = 0;
    }

    if (vbo.size() > LINE_SET_VBO_MAX_VERTEX_COUNT)
    {
        vbo.resize(LINE_SET_VBO_MAX_VERTEX_COUNT);
    }
}
