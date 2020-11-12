// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

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
    // TODO:
}
