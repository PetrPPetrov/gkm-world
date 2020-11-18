// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include <QPainter>
#include <QImage>
#include "global_parameters.h"
#include "genetic_optimization.h"
#include "texture_atlas.h"

TriangleTexture::TriangleTexture(size_t triangle_index_, Vector3u triangle_, unsigned width, unsigned height) : texture(std::make_shared<Texture>(width, height))
{
    triangle_index = triangle_index_;
    triangle = triangle;
}

Texture::Ptr TriangleTexture::getTexture() const
{
    return texture;
}

size_t TriangleTexture::getTriangleIndex() const
{
    return triangle_index;
}

void TriangleTexture::save() const
{
    std::string file_name = "subimage_" + std::to_string(triangle_index) + ".png";
    texture->savePng(file_name.c_str());
}

TextureAtlas::TextureAtlas(const MeshProject::Ptr& mesh_project_, const Mesh::Ptr& mesh_)
{
    mesh_project = mesh_project_;
    mesh = mesh_;
    triangle_textures.reserve(mesh->triangles.size());
}

void TextureAtlas::addTriangleTexture(const TriangleTexture::Ptr& triangle_texture)
{
    triangle_textures.push_back(triangle_texture);
}

void TextureAtlas::build()
{
    auto genetic_algorithm = std::make_shared<GeneticOptimization>(triangle_textures, mesh);

    size_t generation_count = 0;
    Individual::Ptr best = nullptr;
    while (true)
    {
        genetic_algorithm->calculatePenalties();
        genetic_algorithm->sort();
        best = genetic_algorithm->getBest();
        genetic_algorithm->nextGeneration();
        ++generation_count;
        if (generation_count > 10)
        {
            break;
        }
    }

    if (best)
    {
        const int x_lo_in_pixel = static_cast<int>(floor(xl(best->bounding_box) / SCALE) - PROTECTION_OFFSET);
        const int x_hi_in_pixel = static_cast<int>(ceil(xh(best->bounding_box) / SCALE) + PROTECTION_OFFSET);
        const int y_lo_in_pixel = static_cast<int>(floor(yl(best->bounding_box) / SCALE) - PROTECTION_OFFSET);
        const int y_hi_in_pixel = static_cast<int>(ceil(yh(best->bounding_box) / SCALE) + PROTECTION_OFFSET);

        const int x_lo = static_cast<int>(x_lo_in_pixel * SCALE);
        const int x_hi = static_cast<int>(x_hi_in_pixel * SCALE);
        const int y_lo = static_cast<int>(y_lo_in_pixel * SCALE);
        const int y_hi = static_cast<int>(y_hi_in_pixel * SCALE);

        const int x_size_in_pixel = x_hi_in_pixel - x_lo_in_pixel;
        const int y_size_in_pixel = y_hi_in_pixel - y_lo_in_pixel;
        const unsigned texture_atlas_width = static_cast<unsigned>(x_size_in_pixel);
        const unsigned texture_atlas_height = static_cast<unsigned>(y_size_in_pixel);
        texture_atlas = std::make_shared<Texture>(texture_atlas_width, texture_atlas_height);

        // For debug only
        QImage image(texture_atlas_width, texture_atlas_height, QImage::Format::Format_RGB888);
        QPainter painter(&image);
        QPen pen;
        pen.setWidth(1);
        pen.setColor(Qt::red);
        painter.setPen(pen);

        const auto& triangle_texture_information = genetic_algorithm->getTriangleTextureInformation();
        for (auto& gene : best->genotype)
        {
            using namespace boost::polygon;
            using namespace boost::polygon::operators;

            assert(gene.placed);

            TriangleTexture::Ptr triangle_texture = triangle_textures[gene.triangle_texture_index];
            Texture::Ptr texture_fragment = triangle_texture->getTexture();
            auto geometry = triangle_texture_information[gene.triangle_texture_index]->variations[gene.rotation_index]->bloated_polygon;

            NfpPolygonSet real_polygon_set;
            {
                std::vector<NfpPolygon> polygons;
                geometry->get(polygons);
                for (auto& polygon : polygons)
                {
                    move(polygon, HORIZONTAL, x(gene.placement));
                    move(polygon, VERTICAL, y(gene.placement));
                    real_polygon_set += polygon;

                    bool first_point = true;
                    QPainterPath path_to_draw;
                    for (auto& point_it = begin_points(polygon); point_it != end_points(polygon); ++point_it)
                    {
                        if (first_point)
                        {
                            path_to_draw.moveTo((point_it->x() - x_lo) / SCALE, (point_it->y() - y_lo) / SCALE);
                            first_point = false;
                        }
                        else
                        {
                            path_to_draw.lineTo((point_it->x() - x_lo) / SCALE, (point_it->y() - y_lo) / SCALE);
                        }
                    }
                    painter.drawPath(path_to_draw);
                }
            }

            NfpRectange cur_bounding_box;
            extents(cur_bounding_box, real_polygon_set);

            std::vector<NfpPolygon> real_polygons;
            real_polygon_set.get(real_polygons);

            const int cur_x_lo_in_pixel = static_cast<int>(floor(xl(cur_bounding_box) / SCALE));
            const int cur_x_hi_in_pixel = static_cast<int>(ceil(xh(cur_bounding_box) / SCALE));
            const int cur_y_lo_in_pixel = static_cast<int>(floor(yl(cur_bounding_box) / SCALE));
            const int cur_y_hi_in_pixel = static_cast<int>(ceil(yh(cur_bounding_box) / SCALE));

            const int cur_x_lo = static_cast<int>(cur_x_lo_in_pixel  * SCALE);
            const int cur_x_hi = static_cast<int>(cur_x_hi_in_pixel  * SCALE);
            const int cur_y_lo = static_cast<int>(cur_y_lo_in_pixel  * SCALE);
            const int cur_y_hi = static_cast<int>(cur_y_hi_in_pixel  * SCALE);

            const int cur_x_size_in_pixel = cur_x_hi_in_pixel - cur_x_lo_in_pixel;
            const int cur_y_size_in_pixel = cur_y_hi_in_pixel - cur_y_lo_in_pixel;

            const double rotation_step = 360.0 / ROTATION_COUNT * GRAD_TO_RAD;
            Eigen::Rotation2Dd negative_rotation(-rotation_step * gene.rotation_index);
            for (int x = 0; x < cur_x_size_in_pixel; ++x)
            {
                for (int y = 0; y < cur_y_size_in_pixel; ++y)
                {
                    const int cur_x_in_pixel = cur_x_lo_in_pixel + x;
                    const int cur_y_in_pixel = cur_y_lo_in_pixel + y;
                    NfpPoint cur_point0(static_cast<int>(SCALE * cur_x_in_pixel), static_cast<int>(SCALE * cur_y_in_pixel));
                    bool inside = false;
                    for (size_t i = 0; i < real_polygons.size(); ++i)
                    {
                        if (contains(real_polygons[i], cur_point0, true))
                        {
                            inside = true;
                            break;
                        }
                    }
                    if (inside)
                    {
                        Eigen::Vector2d cur_point(cur_point0.x(), cur_point0.y());
                        Eigen::Vector2d cur_relative_point = cur_point - Eigen::Vector2d(gene.placement.x(), gene.placement.y());
                        Eigen::Vector2d original_relative_point = (negative_rotation * cur_relative_point) / SCALE;

                        if (original_relative_point.x() >= 0.0 && original_relative_point.y() >= 0.0 &&
                            original_relative_point.x() <= texture_fragment->getWidth() && original_relative_point.y() <= texture_fragment->getHeight())
                        {
                            std::uint32_t pixel = texture_fragment->getInterpolatedPixel(original_relative_point);
                            texture_atlas->setPixel(cur_x_in_pixel - x_lo_in_pixel, cur_y_in_pixel - y_lo_in_pixel, pixel);
                        }
                    }
                }
            }
        }

        painter.end();
        image.save("atlas.png", "PNG");

        texture_atlas->savePng("texture_atlas.png");
    }
}
