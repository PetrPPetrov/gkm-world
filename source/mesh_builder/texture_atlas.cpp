// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include <QPainter>
#include <QImage>
#include "genetic_optimization.h"
#include "texture_atlas.h"

size_t TriangleTexture::getIndex(unsigned x, unsigned y) const
{
    return (static_cast<size_t>(height) - 1 - y) * width + x;
}

TriangleTexture::TriangleTexture(size_t triangle_index_, Vector3u triangle_, unsigned width_, unsigned height_) : image_data(static_cast<size_t>(width_) * height_)
{
    triangle_index = triangle_index_;
    triangle = triangle;
    width = width_;
    height = height_;
}

unsigned TriangleTexture::getWidth() const
{
    return width;
}

unsigned TriangleTexture::getHeight() const
{
    return height;
}

void TriangleTexture::setPixel(unsigned x, unsigned y, std::uint32_t value)
{
    image_data[getIndex(x, y)] = value;
}

std::uint32_t TriangleTexture::getPixel(unsigned x, unsigned y) const
{
    return image_data[getIndex(x, y)];
}

size_t TriangleTexture::getTriangleIndex() const
{
    return triangle_index;
}

void TriangleTexture::save() const
{
    QImage subimage(reinterpret_cast<const unsigned char*>(&image_data[0]), width, height, QImage::Format_RGB32);
    std::string file_name = "subimage_" + std::to_string(triangle_index) + ".png";
    subimage.save(QString(file_name.c_str()), "PNG");
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
        const double x_lo = xl(best->bounding_box);
        const double x_hi = xh(best->bounding_box);
        const double y_lo = yl(best->bounding_box);
        const double y_hi = yh(best->bounding_box);
        const double x_size = x_hi - x_lo;
        const double y_size = y_hi - y_lo;
        const int texture_atlas_width = static_cast<int>(x_size / SCALE);
        const int texture_atlas_height = static_cast<int>(y_size / SCALE);

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
            auto geometry = triangle_texture_information[gene.triangle_texture_index]->variations[gene.rotation_index]->polygon;
            std::vector<NfpPolygon> polygons;
            geometry->get(polygons);
            for (auto& polygon : polygons)
            {
                move(polygon, HORIZONTAL, x(gene.placement));
                move(polygon, VERTICAL, y(gene.placement));
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
        painter.end();
        image.save("atlas.png", "PNG");
    }
}
