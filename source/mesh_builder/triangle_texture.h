// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include "common.h"
#include "mesh.h"

class TriangleTexture
{
    bool valid_triangle = false;

    MeshProject::Ptr mesh_project;
    Mesh::Ptr new_mesh;
    size_t triangle_index;

    Vector3u triangle;
    Eigen::Vector3d v[3];

    double density;

    Eigen::Vector2d pic0_tri[3];
    Eigen::Vector2d pic1_tri[3];
    ImagePtr photo0;
    ImagePtr photo1;

    Eigen::Vector2d pic0_base;
    Eigen::Vector2d pic0_x_axis_oblique;
    Eigen::Vector2d pic0_y_axis_oblique;

    Eigen::Vector2d pic1_base;
    Eigen::Vector2d pic1_x_axis_oblique;
    Eigen::Vector2d pic1_y_axis_oblique;

    Eigen::Vector3d x_axis_oblique;
    Eigen::Vector3d y_axis_oblique;

    Eigen::Vector2d x_axis_oblique_uv;
    Eigen::Vector2d y_axis_oblique_uv;

    Eigen::Vector3d x_axis;
    Eigen::Vector3d y_axis;

    Eigen::Vector2d uv[3];
    Eigen::Vector2d uv_min;
    Eigen::Vector2d uv_max;

    unsigned new_to_old[3];

    unsigned width;
    unsigned height;

    double uv_width;
    double uv_height;

    double pixel_width_count;
    double pixel_height_count;

    double pixel_width;
    double pixel_height;
    double pixel_width2;
    double pixel_height2;

    double area = 0.0;
    Eigen::Vector2d texture_coordinates[3];

    Eigen::Vector2d solveRectangularCoordinates(const Eigen::Vector3d& vec) const noexcept
    {
        const double u = x_axis.dot(vec);
        const double v = y_axis.dot(vec);
        return Eigen::Vector2d(u, v);
    }

    Eigen::Vector2d solveObliqueCoordinates(const Eigen::Vector2d& uv) const noexcept
    {
        const double t = -uv.y() / y_axis_oblique_uv.y();
        const Eigen::Vector2d temp = uv + y_axis_oblique_uv * t;
        const double u = temp.dot(x_axis_oblique_uv) / x_axis_oblique_uv.squaredNorm();
        const double v = (uv - temp).dot(y_axis_oblique_uv) / y_axis_oblique_uv.squaredNorm();
        // For debug only
        //const Eigen::Vector2d restored_uv = x_axis_oblique_uv * u + y_axis_oblique_uv * v;
        return Eigen::Vector2d(u, v);
    }

    Eigen::Vector2d solveObliqueCoordinates(const Eigen::Vector3d& vec) const noexcept
    {
        const Eigen::Vector2d uv = solveRectangularCoordinates(vec);
        return solveObliqueCoordinates(uv);
    }

    static Eigen::Vector2d calculatePointInObliqueCoordinateSystem(
        const Eigen::Vector2d& uv, const Eigen::Vector2d& base,
        const Eigen::Vector2d& x_axis_oblique, const Eigen::Vector2d& y_axis_oblique) noexcept
    {
        return base + x_axis_oblique * uv.x() + y_axis_oblique * uv.y();
    }

    Eigen::Vector2d calculatePicture0PointInObliqueCoordinateSystem(const Eigen::Vector2d& uv) const noexcept
    {
        return calculatePointInObliqueCoordinateSystem(uv, pic0_base, pic0_x_axis_oblique, pic0_y_axis_oblique);
    }

    Eigen::Vector2d calculatePicture1PointInObliqueCoordinateSystem(const Eigen::Vector2d& uv) const noexcept
    {
        return calculatePointInObliqueCoordinateSystem(uv, pic1_base, pic1_x_axis_oblique, pic1_y_axis_oblique);
    }

    void calculateDensity();
    void prepareAxis();

public:
    typedef std::shared_ptr<TriangleTexture> Ptr;

    TriangleTexture(const MeshProject::Ptr& mesh_project, const Mesh::Ptr& new_mesh, size_t triangle_index);

    size_t getTriangleIndex() const noexcept;
    bool isValid() const noexcept;
    double getDensity() const noexcept;
    void prepareTexture(double total_density);
    double getArea() const noexcept;
    Eigen::Vector2d getTextureCoordinate(unsigned i) const noexcept;

    unsigned getWidth() const;
    unsigned getHeight() const;
    std::uint32_t getPixel(unsigned x, unsigned y) const;
};
