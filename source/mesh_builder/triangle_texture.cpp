// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include <cmath>
#include "task.h"
#include "bilinear_interpolation.h"
#include "triangle_texture.h"
#include "texture_adapter.h"

static inline double sign(const Eigen::Vector2d& p1, const Eigen::Vector2d& p2, const Eigen::Vector2d& p3) noexcept
{
    return (p1.x() - p3.x()) * (p2.y() - p3.y()) - (p2.x() - p3.x()) * (p1.y() - p3.y());
}

static inline bool isPointInTriangle(const Eigen::Vector2d& pt, const Eigen::Vector2d& v1, const Eigen::Vector2d& v2, const Eigen::Vector2d& v3) noexcept
{
    double d1, d2, d3;
    bool has_neg, has_pos;

    d1 = sign(pt, v1, v2);
    d2 = sign(pt, v2, v3);
    d3 = sign(pt, v3, v1);

    has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
    has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

    return !(has_neg && has_pos);
}

static inline double calculateTriangleSquare(const Eigen::Vector2d& v0, const Eigen::Vector2d& v1, const Eigen::Vector2d& v2) noexcept
{
    const double a = (v1 - v0).norm();
    const double b = (v2 - v1).norm();
    const double c = (v2 - v0).norm();
    const double p = (a + b + c) / 2;
    return std::sqrt(p * (p - a) * (p - b) * (p - c));
}

static inline double calculateCellSquare(const Eigen::Vector2d& ll, const Eigen::Vector2d& lr, const Eigen::Vector2d& ul, const Eigen::Vector2d& ur)
{
    double result = 0.0;
    result += calculateTriangleSquare(ll, lr, ur);
    result += calculateTriangleSquare(ll, ur, ul);
    return result;
}

void TriangleTexture::calculateDensity(const MeshProject::Ptr& mesh_project, const Mesh::Ptr& new_mesh)
{
    // TODO: Reduce size of this method
    valid_triangle = false;
    triangle = new_mesh->triangles[triangle_index];

    v[0] = new_mesh->vertices[triangle.x()];
    v[1] = new_mesh->vertices[triangle.y()];
    v[2] = new_mesh->vertices[triangle.z()];

    const double d01 = (v[1] - v[0]).norm();
    const double d12 = (v[2] - v[1]).norm();
    const double d02 = (v[2] - v[0]).norm();

    const size_t old_vertex0_id = new_mesh->new_to_old_vertex_id_map[triangle.x()];
    const size_t old_vertex1_id = new_mesh->new_to_old_vertex_id_map[triangle.y()];
    const size_t old_vertex2_id = new_mesh->new_to_old_vertex_id_map[triangle.z()];

    auto vertex0_info = mesh_project->vertices[old_vertex0_id];
    const unsigned camera0_id = vertex0_info->positions[0]->camera_id;
    const unsigned camera1_id = vertex0_info->positions[1]->camera_id;

    const int v0_p0_x = vertex0_info->positions[0]->x;
    const int v0_p0_y = vertex0_info->positions[0]->y;
    const int v0_p1_x = vertex0_info->positions[1]->x;
    const int v0_p1_y = vertex0_info->positions[1]->y;

    auto vertex1_info = mesh_project->vertices[old_vertex1_id];
    unsigned v1_pos0_index = 0;
    unsigned v1_pos1_index = 1;
    if (vertex1_info->positions[1]->camera_id == camera0_id)
    {
        v1_pos0_index = 1;
        v1_pos1_index = 0;
    }
    const int v1_p0_x = vertex1_info->positions[v1_pos0_index]->x;
    const int v1_p0_y = vertex1_info->positions[v1_pos0_index]->y;
    const int v1_p1_x = vertex1_info->positions[v1_pos1_index]->x;
    const int v1_p1_y = vertex1_info->positions[v1_pos1_index]->y;
    const unsigned v1_camera0_id = vertex1_info->positions[v1_pos0_index]->camera_id;
    const unsigned v1_camera1_id = vertex1_info->positions[v1_pos1_index]->camera_id;
    if (v1_camera0_id != camera0_id || v1_camera1_id != camera1_id)
    {
        // Oops! Different cameras, skipping this triangle
        return;
    }

    auto vertex2_info = mesh_project->vertices[old_vertex2_id];
    unsigned v2_pos0_index = 0;
    unsigned v2_pos1_index = 1;
    if (vertex2_info->positions[1]->camera_id == camera0_id)
    {
        v2_pos0_index = 1;
        v2_pos1_index = 0;
    }
    const int v2_p0_x = vertex2_info->positions[0]->x;
    const int v2_p0_y = vertex2_info->positions[0]->y;
    const int v2_p1_x = vertex2_info->positions[1]->x;
    const int v2_p1_y = vertex2_info->positions[1]->y;
    const unsigned v2_camera0_id = vertex2_info->positions[v2_pos0_index]->camera_id;
    const unsigned v2_camera1_id = vertex2_info->positions[v2_pos1_index]->camera_id;
    if (v2_camera0_id != camera0_id || v2_camera1_id != camera1_id)
    {
        // Oops! Different cameras, skipping this triangle
        return;
    }

    camera0 = projectGetCamera(mesh_project, camera0_id);
    camera1 = projectGetCamera(mesh_project, camera1_id);

    pic0_tri[0] = Eigen::Vector2d(v0_p0_x, v0_p0_y);
    pic0_tri[1] = Eigen::Vector2d(v1_p0_x, v1_p0_y);
    pic0_tri[2] = Eigen::Vector2d(v2_p0_x, v2_p0_y);

    pic1_tri[0] = Eigen::Vector2d(v0_p1_x, v0_p1_y);
    pic1_tri[1] = Eigen::Vector2d(v1_p1_x, v1_p1_y);
    pic1_tri[2] = Eigen::Vector2d(v2_p1_x, v2_p1_y);

    const Eigen::Vector3d v0_p0(v0_p0_x, v0_p0_y, 0);
    const Eigen::Vector3d v0_p1(v0_p1_x, v0_p1_y, 0);

    const Eigen::Vector3d v1_p0(v1_p0_x, v1_p0_y, 0);
    const Eigen::Vector3d v1_p1(v1_p1_x, v1_p1_y, 0);

    const Eigen::Vector3d v2_p0(v2_p0_x, v2_p0_y, 0);
    const Eigen::Vector3d v2_p1(v2_p1_x, v2_p1_y, 0);

    const double d01_p0 = (v1_p0 - v0_p0).norm();
    const double d12_p0 = (v2_p0 - v1_p0).norm();
    const double d02_p0 = (v2_p0 - v0_p0).norm();

    const double d01_p1 = (v1_p1 - v0_p1).norm();
    const double d12_p1 = (v2_p1 - v1_p1).norm();
    const double d02_p1 = (v2_p1 - v0_p1).norm();

    const double picture0_d01_density = d01_p0 / d01;
    const double picture1_d01_density = d01_p1 / d01;

    const double picture0_d12_density = d12_p0 / d12;
    const double picture1_d12_density = d12_p1 / d12;

    const double picture0_d02_density = d02_p0 / d02;
    const double picture1_d02_density = d02_p1 / d02;

    if (mesh_project->triangle_density_mode == EDensityMode::Maximum)
    {
        density = std::max({ picture0_d01_density, picture1_d01_density, picture0_d12_density, picture1_d12_density, picture0_d02_density, picture1_d02_density });
    }
    else
    {
        // Average is default
        density = (picture0_d01_density + picture1_d01_density + picture0_d12_density + picture1_d12_density + picture0_d02_density + picture1_d02_density) / 6.0;
        postProcessDensityMode(density, mesh_project->triangle_density_mode);
    }

    valid_triangle = true;
}

void TriangleTexture::prepareAxis()
{
    const Eigen::Vector3d sv[3] = { v[0], v[1], v[2] };

    const double d01 = (sv[1] - sv[0]).norm();
    const double d12 = (sv[2] - sv[1]).norm();
    const double d02 = (sv[2] - sv[0]).norm();

    if (d01 >= d12 && d01 >= d02)
    {
        new_to_old[0] = 0;
        new_to_old[1] = 1;
        new_to_old[2] = 2;
    }
    else if (d12 >= d01 && d12 >= d02)
    {
        new_to_old[0] = 1;
        new_to_old[1] = 2;
        new_to_old[2] = 0;
    }
    else
    {
        new_to_old[0] = 2;
        new_to_old[1] = 0;
        new_to_old[2] = 1;
    }

    v[0] = sv[new_to_old[0]];
    v[1] = sv[new_to_old[1]];
    v[2] = sv[new_to_old[2]];

    const Vector3u source_triangle = triangle;
    triangle[0] = triangle[new_to_old[0]];
    triangle[1] = triangle[new_to_old[1]];
    triangle[2] = triangle[new_to_old[2]];

    const Eigen::Vector2d source_pic0_tri[3] = { pic0_tri[0], pic0_tri[1], pic0_tri[2] };
    pic0_tri[0] = source_pic0_tri[new_to_old[0]];
    pic0_tri[1] = source_pic0_tri[new_to_old[1]];
    pic0_tri[2] = source_pic0_tri[new_to_old[2]];

    const Eigen::Vector2d source_pic1_tri[3] = { pic1_tri[0], pic1_tri[1], pic1_tri[2] };
    pic1_tri[0] = source_pic1_tri[new_to_old[0]];
    pic1_tri[1] = source_pic1_tri[new_to_old[1]];
    pic1_tri[2] = source_pic1_tri[new_to_old[2]];

    x_axis_oblique = v[1] - v[0];
    y_axis_oblique = v[2] - v[0];

    x_axis = (v[1] - v[0]).normalized();
    const Eigen::Vector3d v2_rel = v[2] - v[0];
    const double v2_x_coord = v2_rel.dot(x_axis);
    const Eigen::Vector3d v2_base = x_axis * v2_x_coord;
    y_axis = (v2_rel - v2_base).normalized();

    x_axis_oblique_uv = Eigen::Vector2d(x_axis_oblique.dot(x_axis), 0);
    y_axis_oblique_uv = solveRectangularCoordinates(y_axis_oblique);

    uv[0] = Eigen::Vector2d(0, 0);
    uv[1] = Eigen::Vector2d((v[1] - v[0]).dot(x_axis), 0);
    uv[2] = Eigen::Vector2d(v2_x_coord, v2_rel.dot(y_axis));
    area = calculateTriangleSquare(uv[0], uv[1], uv[2]);
    uv_min = uv_max = uv[0];

    for (unsigned i = 0; i < 3; ++i)
    {
        const Eigen::Vector2d& cur_uv = uv[i];
        uv_min.x() = std::min(uv_min.x(), cur_uv.x());
        uv_min.y() = std::min(uv_min.y(), cur_uv.y());
        uv_max.x() = std::max(uv_max.x(), cur_uv.x());
        uv_max.y() = std::max(uv_max.y(), cur_uv.y());
    }
}

TriangleTexture::TriangleTexture(const MeshProject::Ptr& mesh_project, const Mesh::Ptr& new_mesh, size_t triangle_index_)
{
    triangle_index = triangle_index_;
    calculateDensity(mesh_project, new_mesh);
}

size_t TriangleTexture::getTriangleIndex() const noexcept
{
    return triangle_index;
}

bool TriangleTexture::isValid() const noexcept
{
    return valid_triangle;
}

double TriangleTexture::getDensity() const noexcept
{
    return density;
}

void TriangleTexture::prepareTexture(double total_density)
{
    prepareAxis();
    density = total_density;

    texture_coordinates[0] = uv[0] * density;
    texture_coordinates[1] = uv[1] * density;
    texture_coordinates[2] = uv[2] * density;

    pic0_base = pic0_tri[0];
    pic0_x_axis_oblique = pic0_tri[1] - pic0_base;
    pic0_y_axis_oblique = pic0_tri[2] - pic0_base;

    pic1_base = pic1_tri[0];
    pic1_x_axis_oblique = pic1_tri[1] - pic1_base;
    pic1_y_axis_oblique = pic1_tri[2] - pic1_base;

    uv_width = uv_max.x() - uv_min.x();
    uv_height = uv_max.y() - uv_min.y();

    pixel_width_count = ceil(uv_width * density);
    pixel_height_count = ceil(uv_height * density);
    width = static_cast<unsigned>(pixel_width_count);
    height = static_cast<unsigned>(pixel_height_count);

    is_pixel_cached.resize(static_cast<size_t>(width) * height, 0);
    cached_image_data.resize(is_pixel_cached.size(), 0);

    pixel_width = uv_width / pixel_width_count;
    pixel_height = uv_height / pixel_height_count;
    pixel_width2 = pixel_width / 2;
    pixel_height2 = pixel_height / 2;
}

double TriangleTexture::getArea() const noexcept
{
    return area;
}

Eigen::Vector2d TriangleTexture::getTextureCoordinate(unsigned i) const noexcept
{
    return texture_coordinates[i];
}

unsigned TriangleTexture::getNewToOld(unsigned i) const noexcept
{
    return new_to_old[i];
}

unsigned TriangleTexture::getWidth() const
{
    return width;
}

unsigned TriangleTexture::getHeight() const
{
    return height;
}

std::uint32_t TriangleTexture::getPixel(unsigned x, unsigned y) const
{
    if (is_pixel_cached[getIndex(x, y)])
    {
        return cached_image_data[getIndex(x, y)];
    }

    const double dx = static_cast<double>(x);
    const double dy = static_cast<double>(y);
    const double u_lo = uv_min.x() + pixel_width * dx;
    const double u_hi = uv_min.x() + pixel_width * (dx + 1);
    const double v_lo = uv_min.y() + pixel_height * dy;
    const double v_hi = uv_min.y() + pixel_height * (dy + 1);
    const double u_center = u_lo + pixel_width2;
    const double v_center = v_lo + pixel_height2;

    const Eigen::Vector2d ll_corner_uv(u_lo, v_lo);
    const Eigen::Vector2d lr_corner_uv(u_hi, v_lo);
    const Eigen::Vector2d ul_corner_uv(u_lo, v_hi);
    const Eigen::Vector2d ur_corner_uv(u_hi, v_hi);

    if (!isPointInTriangle(ll_corner_uv, uv[0], uv[1], uv[2]) &&
        !isPointInTriangle(lr_corner_uv, uv[0], uv[1], uv[2]) &&
        !isPointInTriangle(ul_corner_uv, uv[0], uv[1], uv[2]) &&
        !isPointInTriangle(ur_corner_uv, uv[0], uv[1], uv[2]))
    {
        const std::uint32_t white_pixel = 0xffffffff;
        is_pixel_cached[getIndex(x, y)] = 1;
        cached_image_data[getIndex(x, y)] = white_pixel;
        return white_pixel;
    }

    const Eigen::Vector2d ll_corner = solveObliqueCoordinates(ll_corner_uv);
    const Eigen::Vector2d lr_corner = solveObliqueCoordinates(lr_corner_uv);
    const Eigen::Vector2d ul_corner = solveObliqueCoordinates(ul_corner_uv);
    const Eigen::Vector2d ur_corner = solveObliqueCoordinates(ur_corner_uv);
    const Eigen::Vector2d center = solveObliqueCoordinates(Eigen::Vector2d(u_center, v_center));

    const Eigen::Vector2d ll_corner_p0 = calculatePicture0PointInObliqueCoordinateSystem(ll_corner);
    const Eigen::Vector2d lr_corner_p0 = calculatePicture0PointInObliqueCoordinateSystem(lr_corner);
    const Eigen::Vector2d ul_corner_p0 = calculatePicture0PointInObliqueCoordinateSystem(ul_corner);
    const Eigen::Vector2d ur_corner_p0 = calculatePicture0PointInObliqueCoordinateSystem(ur_corner);
    const Eigen::Vector2d center_p0 = calculatePicture0PointInObliqueCoordinateSystem(center);
    const double pixel_p0_square = calculateCellSquare(ll_corner_p0, lr_corner_p0, ul_corner_p0, ur_corner_p0);

    const Eigen::Vector2d ll_corner_p1 = calculatePicture1PointInObliqueCoordinateSystem(ll_corner);
    const Eigen::Vector2d lr_corner_p1 = calculatePicture1PointInObliqueCoordinateSystem(lr_corner);
    const Eigen::Vector2d ul_corner_p1 = calculatePicture1PointInObliqueCoordinateSystem(ul_corner);
    const Eigen::Vector2d ur_corner_p1 = calculatePicture1PointInObliqueCoordinateSystem(ur_corner);
    const Eigen::Vector2d center_p1 = calculatePicture1PointInObliqueCoordinateSystem(center);
    const double pixel_p1_square = calculateCellSquare(ll_corner_p1, lr_corner_p1, ul_corner_p1, ur_corner_p1);

    // Select picture where the current pixel has a higher square
    const Camera::Ptr selected_camera = (pixel_p0_square > pixel_p1_square) ? camera0 : camera1;
    const Eigen::Vector2d& selected_center = (pixel_p0_square > pixel_p1_square) ? center_p0 : center_p1;

    std::uint32_t interpolated_pixel = getInterpolatedPixel(std::make_shared<TextureAdapter>(selected_camera), selected_center);

    is_pixel_cached[getIndex(x, y)] = 1;
    cached_image_data[getIndex(x, y)] = interpolated_pixel;

    return interpolated_pixel;
}
