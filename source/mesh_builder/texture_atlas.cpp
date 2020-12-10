// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include <QPainter>
#include <QImage>
#include "global_parameters.h"
#include "genetic_optimization.h"
#include "texture_atlas.h"
#include "task.h"

TriangleTexture::TriangleTexture(size_t triangle_index_, unsigned width, unsigned height) : texture(std::make_shared<Texture>(width, height))
{
    triangle_index = triangle_index_;
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
    mesh->texture_atlas = texture_atlas;
}

void TextureAtlas::addTriangleTexture(const TriangleTexture::Ptr& triangle_texture)
{
    triangle_textures.push_back(triangle_texture);
}

void TextureAtlas::build()
{
    auto genetic_algorithm = std::make_shared<GeneticOptimization>(triangle_textures, mesh, mesh_project);

    Job job(2, "Generating texture atlas...");

    Individual::Ptr best = nullptr;
    {
        const size_t GENERATION_NUMBER = 10;
        Job job(GENERATION_NUMBER, "Nesting texture fragments...");
        for (size_t generation_count = 0; generation_count < GENERATION_NUMBER; ++generation_count)
        {
            Job job(2);
            genetic_algorithm->calculatePenalties();
            genetic_algorithm->sort();
            best = genetic_algorithm->getBest();
            genetic_algorithm->nextGeneration();
        }
    }

    if (best)
    {
        Job job(3);

        const double rotation_step = 360.0 / mesh_project->rotation_count * GRAD_TO_RAD;
        mesh->triangle_tex_coords.resize(mesh->triangles.size() * 3);

        const int x_lo_in_pixel = static_cast<int>(floor(xl(best->bounding_box) / mesh_project->scale) - mesh_project->protection_offset);
        const int x_hi_in_pixel = static_cast<int>(ceil(xh(best->bounding_box) / mesh_project->scale) + mesh_project->protection_offset);
        const int y_lo_in_pixel = static_cast<int>(floor(yl(best->bounding_box) / mesh_project->scale) - mesh_project->protection_offset);
        const int y_hi_in_pixel = static_cast<int>(ceil(yh(best->bounding_box) / mesh_project->scale) + mesh_project->protection_offset);

        const int x_size_in_pixel = x_hi_in_pixel - x_lo_in_pixel;
        const int y_size_in_pixel = y_hi_in_pixel - y_lo_in_pixel;

        const unsigned texture_atlas_width = static_cast<unsigned>(x_size_in_pixel);
        const unsigned texture_atlas_height = static_cast<unsigned>(y_size_in_pixel);

        texture_atlas = std::make_shared<Texture>(texture_atlas_width, texture_atlas_height);
        mesh->texture_atlas = texture_atlas;

        const int x_lo = static_cast<int>(x_lo_in_pixel * mesh_project->scale);
        const int x_hi = static_cast<int>(x_hi_in_pixel * mesh_project->scale);
        const int y_lo = static_cast<int>(y_lo_in_pixel * mesh_project->scale);
        const int y_hi = static_cast<int>(y_hi_in_pixel * mesh_project->scale);

        const int x_size = x_hi - x_lo;
        const int y_size = y_hi - y_lo;

        // For debug only
        //QImage image(texture_atlas_width, texture_atlas_height, QImage::Format::Format_RGB888);
        //QPainter painter(&image);
        //QPen pen;
        //pen.setWidth(1);
        //pen.setColor(Qt::red);
        //painter.setPen(pen);

        const auto& triangle_texture_information = genetic_algorithm->getTriangleTextureInformation();

        {
            Job job(best->genotype.size(), "Filling texture atlas...");
            for (auto& gene : best->genotype)
            {
                using namespace boost::polygon;
                using namespace boost::polygon::operators;

                assert(gene.placed);

                TriangleTexture::Ptr triangle_texture = triangle_textures[gene.triangle_texture_index];
                Texture::Ptr texture_fragment = triangle_texture->getTexture();
                auto bloated_geometry = triangle_texture_information[gene.triangle_texture_index]->variations[gene.rotation_index]->bloated_geometry;

                NfpPolygonSet real_polygon_set;
                {
                    for (auto& polygon : bloated_geometry->polygons)
                    {
                        NfpPolygon new_polygon = polygon;
                        move(new_polygon, HORIZONTAL, x(gene.placement));
                        move(new_polygon, VERTICAL, y(gene.placement));
                        real_polygon_set += new_polygon;

                        //bool first_point = true;
                        //QPainterPath path_to_draw;
                        //for (auto& point_it = begin_points(new_polygon); point_it != end_points(new_polygon); ++point_it)
                        //{
                        //    if (first_point)
                        //    {
                        //        path_to_draw.moveTo((point_it->x() - x_lo) / mesh_project->scale, (point_it->y() - y_lo) / mesh_project->scale);
                        //        first_point = false;
                        //    }
                        //    else
                        //    {
                        //        path_to_draw.lineTo((point_it->x() - x_lo) / mesh_project->scale, (point_it->y() - y_lo) / mesh_project->scale);
                        //    }
                        //}
                        //painter.drawPath(path_to_draw);
                    }
                }

                NfpRectange cur_bounding_box;
                extents(cur_bounding_box, real_polygon_set);

                std::vector<NfpPolygon> real_polygons;
                real_polygon_set.get(real_polygons);

                const int cur_x_lo_in_pixel = static_cast<int>(floor(xl(cur_bounding_box) / mesh_project->scale));
                const int cur_x_hi_in_pixel = static_cast<int>(ceil(xh(cur_bounding_box) / mesh_project->scale));
                const int cur_y_lo_in_pixel = static_cast<int>(floor(yl(cur_bounding_box) / mesh_project->scale));
                const int cur_y_hi_in_pixel = static_cast<int>(ceil(yh(cur_bounding_box) / mesh_project->scale));

                const int cur_x_lo = static_cast<int>(cur_x_lo_in_pixel * mesh_project->scale);
                const int cur_x_hi = static_cast<int>(cur_x_hi_in_pixel * mesh_project->scale);
                const int cur_y_lo = static_cast<int>(cur_y_lo_in_pixel * mesh_project->scale);
                const int cur_y_hi = static_cast<int>(cur_y_hi_in_pixel * mesh_project->scale);

                const int cur_x_size_in_pixel = cur_x_hi_in_pixel - cur_x_lo_in_pixel;
                const int cur_y_size_in_pixel = cur_y_hi_in_pixel - cur_y_lo_in_pixel;

                Eigen::Rotation2Dd negative_rotation(-rotation_step * gene.rotation_index);

                Job job(static_cast<size_t>(cur_x_size_in_pixel) * cur_y_size_in_pixel, "Copying texture fragment to atlas...");
                for (int x = 0; x < cur_x_size_in_pixel; ++x)
                {
                    for (int y = 0; y < cur_y_size_in_pixel; ++y)
                    {
                        const int cur_x_in_pixel = cur_x_lo_in_pixel + x;
                        const int cur_y_in_pixel = cur_y_lo_in_pixel + y;
                        NfpPoint cur_point0(static_cast<int>(mesh_project->scale * cur_x_in_pixel), static_cast<int>(mesh_project->scale * cur_y_in_pixel));
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
                            //if (gene.triangle_texture_index == 0)
                            //{
                            //    texture_atlas->setPixel(cur_x_in_pixel - x_lo_in_pixel, cur_y_in_pixel - y_lo_in_pixel, 0xffff0000);
                            //}
                            //else
                            //{
                            //    texture_atlas->setPixel(cur_x_in_pixel - x_lo_in_pixel, cur_y_in_pixel - y_lo_in_pixel, 0xff00ff00);
                            //}
                            Eigen::Vector2d cur_point(cur_point0.x(), cur_point0.y());
                            Eigen::Vector2d cur_relative_point = cur_point - Eigen::Vector2d(gene.placement.x(), gene.placement.y());
                            Eigen::Vector2d original_relative_point0 = negative_rotation * cur_relative_point;
                            Eigen::Vector2d original_relative_point = original_relative_point0 / mesh_project->scale;

                            std::uint32_t pixel = texture_fragment->getInterpolatedPixel(original_relative_point);
                            texture_atlas->setPixel(cur_x_in_pixel - x_lo_in_pixel, cur_y_in_pixel - y_lo_in_pixel, pixel);
                        }
                        job.step();
                    }
                }
            }
        }

        {
            // Correct texture coordinates
            Job job(best->genotype.size(), "Correcting texture coordinates...");
            for (auto& gene : best->genotype)
            {
                TriangleTexture::Ptr triangle_texture = triangle_textures[gene.triangle_texture_index];
                Eigen::Rotation2Dd positive_rotation(rotation_step * gene.rotation_index);

                for (unsigned i = 0; i < 3; ++i)
                {
                    const Eigen::Vector2d tex_coord = positive_rotation * triangle_texture->texture_coordinates[i];
                    const int cur_x = x(gene.placement) + static_cast<int>(mesh_project->scale * tex_coord.x());
                    const int cur_y = y(gene.placement) + static_cast<int>(mesh_project->scale * tex_coord.y());
                    const double u = (static_cast<double>(cur_x) - x_lo) / x_size;
                    const double v = (static_cast<double>(cur_y) - y_lo) / y_size;
                    mesh->triangle_tex_coords[triangle_texture->getTriangleIndex() * 3 + triangle_texture->new_to_old[i]] = Eigen::Vector2d(u, v);
                }
                job.step();
            }
        }

        if (texture_atlas->getWidth() > mesh_project->max_texture_size ||
            texture_atlas->getHeight() > mesh_project->max_texture_size)
        {
            const double ratio_x = static_cast<double>(mesh_project->max_texture_size) / texture_atlas->getWidth();
            const double ratio_y = static_cast<double>(mesh_project->max_texture_size) / texture_atlas->getHeight();
            unsigned new_width;
            unsigned new_height;
            if (ratio_x < ratio_y)
            {
                new_width = mesh_project->max_texture_size;
                new_height = static_cast<unsigned>(ceil(ratio_x * texture_atlas->getHeight() - 0.5));
                // TODO: Correct texture coordinates because new texture aspect ratio is not the same as previous
                // due rounding new_height
            }
            else
            {
                new_height = mesh_project->max_texture_size;
                new_width = static_cast<unsigned>(ceil(ratio_y * texture_atlas->getWidth() - 0.5));
                // TODO: Correct texture coordinates because new texture aspect ratio is not the same as previous
                // due rounding new_width
            }
            mesh->texture_atlas = texture_atlas = texture_atlas->createNewSize(new_width, new_height);
        }

        //painter.end();
        //image.save("atlas.png", "PNG");

        // For debug only
        //ImagePtr image = texture_atlas->getQImage();
        //QPainter painter(image.get());

        //bool f = true;
        //for (auto& gene : best->genotype)
        //{
        //    if (f)
        //    {
        //        QPen pen;
        //        pen.setWidth(3);
        //        pen.setColor(Qt::red);
        //        painter.setPen(pen);
        //        f = false;
        //    }
        //    else
        //    {
        //        QPen pen;
        //        pen.setWidth(3);
        //        pen.setColor(Qt::blue);
        //        painter.setPen(pen);
        //    }

        //    TriangleTexture::Ptr triangle_texture = triangle_textures[gene.triangle_texture_index];
        //    Eigen::Rotation2Dd positive_rotation(rotation_step* gene.rotation_index);

        //    QPainterPath path_to_draw;
        //    for (unsigned i = 0; i < 4; ++i)
        //    {
        //        const Eigen::Vector2d tex_coord = positive_rotation * triangle_texture->texture_coordinates[i % 3];
        //        const int cur_x = x(gene.placement) + static_cast<int>(mesh_project->scale * tex_coord.x());
        //        const int cur_y = y(gene.placement) + static_cast<int>(mesh_project->scale * tex_coord.y());
        //        const double u = (static_cast<double>(cur_x) - x_lo) / x_size;
        //        const double v = (static_cast<double>(cur_y) - y_lo) / y_size;
        //        if (i == 0)
        //        {
        //            path_to_draw.moveTo(u * texture_atlas_width, (1.0 - v) * texture_atlas_height);
        //        }
        //        else
        //        {
        //            path_to_draw.lineTo(u * texture_atlas_width, (1.0 - v) * texture_atlas_height);
        //        }
        //    }
        //    painter.drawPath(path_to_draw);
        //}
        //image->save(QString("texture_debug_atlas.png"), "PNG");
    }
}
