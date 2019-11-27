// Copyright 2018-2019 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include "Eigen/Eigen"
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/box.hpp>
#include "global_parameters.h"

typedef boost::geometry::model::point<double, 2, boost::geometry::cs::cartesian> point2d_t;
typedef boost::geometry::model::point<double, 3, boost::geometry::cs::cartesian> point3d_t;
typedef boost::geometry::model::box<point2d_t> box2d_t;

typedef boost::geometry::model::point<std::int32_t, 2, boost::geometry::cs::cartesian> point2i_t;
typedef boost::geometry::model::box<point2i_t> box2i_t;

inline bool inside(double x_pos, double y_pos, const box2d_t& bounding_box)
{
    if (x_pos >= bounding_box.min_corner().get<0>() &&
        y_pos >= bounding_box.min_corner().get<1>() &&
        x_pos < bounding_box.max_corner().get<0>() &&
        y_pos < bounding_box.max_corner().get<1>())
    {
        return true;
    }
    return false;
}

inline bool inside(double x_pos, double y_pos, const box2i_t& bounding_box)
{
    if (x_pos >= bounding_box.min_corner().get<0>() * CELL_UNIT_SIZE &&
        y_pos >= bounding_box.min_corner().get<1>() * CELL_UNIT_SIZE &&
        x_pos < bounding_box.max_corner().get<0>() * CELL_UNIT_SIZE &&
        y_pos < bounding_box.max_corner().get<1>() * CELL_UNIT_SIZE)
    {
        return true;
    }
    return false;
}

inline Eigen::Vector2d b2e(const point2d_t& point)
{
    return Eigen::Vector2d(point.get<0>(), point.get<1>());
}

inline Eigen::Vector2d b2e(const point2i_t& point)
{
    return Eigen::Vector2d(point.get<0>() * CELL_UNIT_SIZE, point.get<1>() * CELL_UNIT_SIZE);
}

inline point2d_t e2b(const Eigen::Vector2d& point)
{
    return point2d_t(point.x(), point.y());
}
