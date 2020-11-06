// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include <cstdint>
#include <cmath>
#include <vector>
#include <QRgb>
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

typedef Eigen::Matrix<std::uint32_t, Triangle::vertex_count, 1> Vector3u;

struct Mesh
{
    typedef std::shared_ptr<Mesh> Ptr;

    std::vector<Eigen::Vector3d> vertices;
    std::vector<size_t> old_to_new_vertex_id_map;
    std::vector<size_t> new_to_old_vertex_id_map;

    std::vector<Vector3u> triangles;
    //std::vector<size_t> old_to_new_triangle_id_map;
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
    for (auto& triangle : mesh->triangles)
    {
        file_out << "f " << triangle.x() + 1 << " " << triangle.y() + 1 << " " << triangle.z() + 1 << "\n";
    }
}

void calculateVertices(const MeshProject::Ptr& mesh_project, const Mesh::Ptr& new_mesh)
{
    const size_t source_vertex_count = mesh_project->vertices.size();
    new_mesh->vertices.reserve(source_vertex_count);
    new_mesh->old_to_new_vertex_id_map.resize(source_vertex_count, source_vertex_count);
    new_mesh->new_to_old_vertex_id_map.resize(source_vertex_count, source_vertex_count);
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
    }
}

void mapTriangles(const MeshProject::Ptr& mesh_project, const Mesh::Ptr& new_mesh)
{
    const size_t source_vertex_count = mesh_project->vertices.size();
    const size_t source_triangle_count = mesh_project->triangles.size();
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
    }
}

static inline double calculateTriangleSquare(const Eigen::Vector2d& v0, const Eigen::Vector2d& v1, const Eigen::Vector2d& v2)
{
    Eigen::Vector2d base_vec = v1 - v0;
    const double base = base_vec.norm();
    base_vec.normalize();

    const Eigen::Vector2d v2_rel = v2 - v0;
    const double v2_base_distance = v2_rel.dot(base_vec);
    const Eigen::Vector2d v2_base_point = base_vec * v2_base_distance;
    const double h = (v2_rel - v2_base_point).norm();
    return base * h / 2;
}

static inline double calculateCellSquare(const Eigen::Vector2d& ll, const Eigen::Vector2d& lr, const Eigen::Vector2d& ul, const Eigen::Vector2d& ur)
{
    double result = 0.0;
    result += calculateTriangleSquare(ll, lr, ur);
    result += calculateTriangleSquare(ll, ur, ul);
    return result;
}

static inline double sign(const Eigen::Vector2d& p1, const Eigen::Vector2d& p2, const Eigen::Vector2d& p3)
{
    return (p1.x() - p3.x()) * (p2.y() - p3.y()) - (p2.x() - p3.x()) * (p1.y() - p3.y());
}

static inline bool isPointInTriangle(const Eigen::Vector2d& pt, const Eigen::Vector2d& v1, const Eigen::Vector2d& v2, const Eigen::Vector2d& v3)
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

static inline QRgb getPixel(const ImagePtr& image, const Eigen::Vector2d& source_pixel)
{
    Eigen::Vector2d pixel = source_pixel;
    bool increase_right = false;
    bool decrease_left = false;
    bool increase_top = false;
    bool decrease_bottom = false;

    if (pixel.x() <= 0.0)
    {
        pixel.x() = 0.0;
        increase_right = true;
    }
    if (pixel.y() < 0.0)
    {
        pixel.y() = 0.0;
        increase_top = true;
    }
    if (pixel.x() >= image->width())
    {
        pixel.x() = image->width();
        decrease_left = true;
    }
    if (pixel.y() >= image->height())
    {
        pixel.y() = image->height();
        decrease_bottom = true;
    }

    double left = floor(pixel.x());
    double right = ceil(pixel.x());
    double bottom = floor(pixel.y());
    double top = ceil(pixel.y());

    if (increase_right)
    {
        right += 1.0;
    }
    else if (decrease_left)
    {
        left -= 1.0;
    }

    if (increase_top)
    {
        top += 1.0;
    }
    else if (decrease_bottom)
    {
        bottom -= 1.0;
    }

    const double distance_to_left = pixel.x() - left;
    const double distance_to_right = right - pixel.x();
    const double distance_to_bottom = pixel.y() - bottom;
    const double distance_to_top = top - pixel.y();

    const double lower_left_square = distance_to_left * distance_to_bottom;
    const double lower_right_square = distance_to_right * distance_to_bottom;
    const double upper_left_square = distance_to_left * distance_to_top;
    const double upper_right_square = distance_to_right * distance_to_top;

    QRgb ll_pixel = image->pixel(static_cast<int>(left), static_cast<int>(bottom));
    QRgb lr_pixel = image->pixel(static_cast<int>(right), static_cast<int>(bottom));
    QRgb ul_pixel = image->pixel(static_cast<int>(left), static_cast<int>(top));
    QRgb ur_pixel = image->pixel(static_cast<int>(right), static_cast<int>(top));

    Eigen::Vector4d lower_left_pixel(qRed(ll_pixel) / 255.0, qGreen(ll_pixel) / 255.0, qBlue(ll_pixel) / 255.0, qAlpha(ll_pixel) / 255.0);
    Eigen::Vector4d lower_right_pixel(qRed(lr_pixel) / 255.0, qGreen(lr_pixel) / 255.0, qBlue(lr_pixel) / 255.0, qAlpha(lr_pixel) / 255.0);
    Eigen::Vector4d upper_left_pixel(qRed(ul_pixel) / 255.0, qGreen(ul_pixel) / 255.0, qBlue(ul_pixel) / 255.0, qAlpha(ul_pixel) / 255.0);
    Eigen::Vector4d upper_right_pixel(qRed(ur_pixel) / 255.0, qGreen(ur_pixel) / 255.0, qBlue(ur_pixel) / 255.0, qAlpha(ur_pixel) / 255.0);

    const double lower_left_weight = upper_right_square;
    const double lower_right_weight = upper_left_square;
    const double upper_left_weight = lower_right_square;
    const double upper_right_weight = lower_left_square;

    Eigen::Vector4d interpolated_pixel = lower_left_pixel * lower_left_weight + lower_right_pixel * lower_right_weight + upper_left_pixel * upper_left_weight + upper_right_pixel * upper_right_weight;

    return qRgba(
        static_cast<int>(interpolated_pixel.x() * 255.0),
        static_cast<int>(interpolated_pixel.y() * 255.0),
        static_cast<int>(interpolated_pixel.z() * 255.0),
        static_cast<int>(interpolated_pixel.w() * 255.0));
}

class PictureTriangle
{
    Eigen::Vector2d v[3];

    Eigen::Vector2d x_axis_oblique;
    Eigen::Vector2d y_axis_oblique;
    ImagePtr image;

public:
    PictureTriangle(const Eigen::Vector2d sv[3], const unsigned new_to_old[3], const ImagePtr& image_)
    {
        v[0] = sv[new_to_old[0]];
        v[1] = sv[new_to_old[1]];
        v[2] = sv[new_to_old[2]];

        x_axis_oblique = v[1] - v[0];
        y_axis_oblique = v[2] - v[0];
        image = image_;
    }

    Eigen::Vector2d calculatePointInObliqueCoordinateSystem(const Eigen::Vector2d& uv) const
    {
        return v[0] + x_axis_oblique * uv.x() + y_axis_oblique * uv.y();
    }

    ImagePtr getImage() const
    {
        return image;
    }
};

class ComputationTriangle
{
    Eigen::Vector3d v[3];

    Eigen::Vector3d x_axis_oblique;
    Eigen::Vector3d y_axis_oblique;

    Eigen::Vector2d x_axis_oblique_uv;
    Eigen::Vector2d y_axis_oblique_uv;

    Eigen::Vector3d x_axis;
    Eigen::Vector3d y_axis;

    Eigen::Vector2d uv[3];
    Eigen::Vector2d uv_min;
    Eigen::Vector2d uv_max;

    Eigen::Vector2d solveRectangularCoordinates(const Eigen::Vector3d& vec) const
    {
        const double u = x_axis.dot(vec);
        const double v = y_axis.dot(vec);
        return Eigen::Vector2d(u, v);
    }

    Eigen::Vector2d solveObliqueCoordinates(const Eigen::Vector2d& uv) const
    {
        const double v = -uv.y() / y_axis_oblique_uv.y();
        const Eigen::Vector2d temp = uv + y_axis_oblique_uv * v;
        const double u = temp.norm();
        return Eigen::Vector2d(u, v);
    }

    Eigen::Vector2d solveObliqueCoordinates(const Eigen::Vector3d& vec) const
    {
        const Eigen::Vector2d uv = solveRectangularCoordinates(vec);
        return solveObliqueCoordinates(uv);
    }

public:
    unsigned new_to_old[3];

    ComputationTriangle(const Eigen::Vector3d sv[3])
    {
        const double d01 = (sv[1] - sv[0]).norm();
        const double d12 = (sv[2] - sv[1]).norm();
        const double d02 = (sv[2] - sv[0]).norm();

        if (d01 >= d12 && d01 >= d02)
        {
            v[0] = sv[0];
            v[1] = sv[1];
            v[2] = sv[2];
            new_to_old[0] = 0;
            new_to_old[1] = 1;
            new_to_old[2] = 2;
        }
        else if (d12 >= d01 && d12 >= d02)
        {
            v[0] = sv[1];
            v[1] = sv[2];
            v[2] = sv[0];
            new_to_old[0] = 1;
            new_to_old[1] = 2;
            new_to_old[2] = 0;
        }
        else
        {
            v[0] = sv[2];
            v[1] = sv[0];
            v[2] = sv[1];
            new_to_old[0] = 2;
            new_to_old[1] = 0;
            new_to_old[2] = 1;
        }

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
        uv_min = uv[0];
        uv_max = uv[0];

        for (unsigned i = 0; i < 3; ++i)
        {
            const Eigen::Vector2d cur_uv = uv[i];
            if (cur_uv.x() < uv_min.x())
            {
                uv_min.x() = cur_uv.x();
            }
            if (cur_uv.y() < uv_min.y())
            {
                uv_min.y() = cur_uv.y();
            }
            if (cur_uv.x() > uv_max.x())
            {
                uv_max.x() = cur_uv.x();
            }
            if (cur_uv.y() > uv_max.y())
            {
                uv_max.y() = cur_uv.y();
            }
        }
    }

    void build(double density, const PictureTriangle& picture0_triangle, const PictureTriangle& picture1_triangle)
    {
        const double uv_width = uv_max.x() - uv_min.x();
        const double uv_height = uv_max.y() - uv_min.y();

        const double pixel_width_count = ceil(uv_width * density);
        const double pixel_height_count = ceil(uv_height * density);
        const unsigned pixel_count_w = static_cast<unsigned>(pixel_width_count);
        const unsigned pixel_count_h = static_cast<unsigned>(pixel_height_count);

        std::vector<std::uint32_t> subimage_data(pixel_count_w * pixel_count_h);

        const double pixel_width = uv_width / pixel_width_count;
        const double pixel_height = uv_height / pixel_height_count;
        const double pixel_width2 = pixel_width / 2;
        const double pixel_height2 = pixel_height / 2;

        for (unsigned x = 0; x < pixel_count_w; ++x)
        {
            for (unsigned y = 0; y < pixel_count_h; ++y)
            {
                const double u_lo = uv_min.x() + pixel_width * x;
                const double u_hi = uv_min.x() + pixel_width * (x + 1);
                const double v_lo = uv_min.y() + pixel_height * y;
                const double v_hi = uv_min.y() + pixel_height * (y + 1);
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
                    continue;
                }

                const Eigen::Vector2d ll_corner = solveObliqueCoordinates(ll_corner_uv);
                const Eigen::Vector2d lr_corner = solveObliqueCoordinates(lr_corner_uv);
                const Eigen::Vector2d ul_corner = solveObliqueCoordinates(ul_corner_uv);
                const Eigen::Vector2d ur_corner = solveObliqueCoordinates(ur_corner_uv);
                const Eigen::Vector2d center = solveObliqueCoordinates(Eigen::Vector2d(u_center, v_center));

                const Eigen::Vector2d ll_corner_p0 = picture0_triangle.calculatePointInObliqueCoordinateSystem(ll_corner);
                const Eigen::Vector2d lr_corner_p0 = picture0_triangle.calculatePointInObliqueCoordinateSystem(lr_corner);
                const Eigen::Vector2d ul_corner_p0 = picture0_triangle.calculatePointInObliqueCoordinateSystem(ul_corner);
                const Eigen::Vector2d ur_corner_p0 = picture0_triangle.calculatePointInObliqueCoordinateSystem(ur_corner);
                const Eigen::Vector2d center_p0 = picture0_triangle.calculatePointInObliqueCoordinateSystem(center);
                const double pixel_p0_square = calculateCellSquare(ll_corner_p0, lr_corner_p0, ul_corner_p0, ur_corner_p0);

                const Eigen::Vector2d ll_corner_p1 = picture1_triangle.calculatePointInObliqueCoordinateSystem(ll_corner);
                const Eigen::Vector2d lr_corner_p1 = picture1_triangle.calculatePointInObliqueCoordinateSystem(lr_corner);
                const Eigen::Vector2d ul_corner_p1 = picture1_triangle.calculatePointInObliqueCoordinateSystem(ul_corner);
                const Eigen::Vector2d ur_corner_p1 = picture1_triangle.calculatePointInObliqueCoordinateSystem(ur_corner);
                const Eigen::Vector2d center_p1 = picture1_triangle.calculatePointInObliqueCoordinateSystem(center);
                const double pixel_p1_square = calculateCellSquare(ll_corner_p1, lr_corner_p1, ul_corner_p1, ur_corner_p1);

                // Select picture where the current pixel has a higher square
                const PictureTriangle& selected_picture_triange = (pixel_p0_square > pixel_p1_square) ? picture0_triangle : picture1_triangle;
                const Eigen::Vector2d& selected_center = (pixel_p0_square > pixel_p1_square) ? center_p0 : center_p1;

                QRgb new_pixel = getPixel(selected_picture_triange.getImage(), selected_center);
                subimage_data[y * pixel_count_w + x] = new_pixel;
            }
        }

        QImage subimage(reinterpret_cast<const unsigned char*>(&subimage_data[0]), pixel_count_w, pixel_count_h, QImage::Format_ARGB32);
        static int image_counter = 0;
        std::string file_name = "subimage_" + std::to_string(image_counter++) + ".png";
        subimage.save(QString(file_name.c_str()), "PNG");
    }
};

void buildTexture(const MeshProject::Ptr& mesh_project, const Mesh::Ptr& new_mesh)
{
    const size_t new_triangle_count = new_mesh->triangles.size();
    for (size_t i = 0; i < new_triangle_count; ++i)
    {
        Vector3u cur_triangle = new_mesh->triangles[i];

        Eigen::Vector3d v[3];
        v[0] = new_mesh->vertices[cur_triangle.x()];
        v[1] = new_mesh->vertices[cur_triangle.y()];
        v[2] = new_mesh->vertices[cur_triangle.z()];

        ComputationTriangle comp_triangle(v);

        const double d01 = (v[1] - v[0]).norm();
        const double d12 = (v[2] - v[1]).norm();
        const double d02 = (v[2] - v[0]).norm();

        const size_t old_vertex0_id = new_mesh->new_to_old_vertex_id_map[cur_triangle.x()];
        const size_t old_vertex1_id = new_mesh->new_to_old_vertex_id_map[cur_triangle.y()];
        const size_t old_vertex2_id = new_mesh->new_to_old_vertex_id_map[cur_triangle.z()];

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
            continue;
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
            continue;
        }

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

        //const double density = std::max({ picture0_d01_density, picture1_d01_density, picture0_d12_density, picture1_d12_density, picture0_d02_density, picture1_d02_density });
        //const double density = (picture0_d01_density + picture1_d01_density + picture0_d12_density + picture1_d12_density + picture0_d02_density + picture1_d02_density) / 6.0;
        const double density = 256.0;

        Eigen::Vector2d picture0_tr[3];
        picture0_tr[0] = Eigen::Vector2d(v0_p0_x, v0_p0_y);
        picture0_tr[1] = Eigen::Vector2d(v1_p0_x, v1_p0_y);
        picture0_tr[2] = Eigen::Vector2d(v2_p0_x, v2_p0_y);

        Eigen::Vector2d picture1_tr[3];
        picture1_tr[0] = Eigen::Vector2d(v0_p1_x, v0_p1_y);
        picture1_tr[1] = Eigen::Vector2d(v1_p1_x, v1_p1_y);
        picture1_tr[2] = Eigen::Vector2d(v2_p1_x, v2_p1_y);

        Camera::Ptr camera0 = projectGetCamera(mesh_project, camera0_id);
        Camera::Ptr camera1 = projectGetCamera(mesh_project, camera1_id);

        PictureTriangle picture0_triangle(picture0_tr, comp_triangle.new_to_old, camera0->photo_image);
        PictureTriangle picture1_triangle(picture1_tr, comp_triangle.new_to_old, camera1->photo_image);

        comp_triangle.build(density, picture0_triangle, picture1_triangle);
    }
}

void buildMesh(const MeshProject::Ptr& mesh_project)
{
    Mesh::Ptr new_mesh = std::make_shared<Mesh>();

    calculateVertices(mesh_project, new_mesh);
    mapTriangles(mesh_project, new_mesh);
    buildTexture(mesh_project, new_mesh);

    saveMeshObj(mesh_project->output_file_name, new_mesh, mesh_project->file_name);
}
