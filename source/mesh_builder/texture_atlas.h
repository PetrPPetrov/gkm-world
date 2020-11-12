// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include "common.h"
#include "mesh_project.h"
#include "mesh.h"

class TriangleTexture
{
    std::vector<std::uint32_t> image_data;
    unsigned width;
    unsigned height;
    size_t triangle_index;
    Vector3u triangle;

    size_t getIndex(unsigned x, unsigned y) const;

public:
    typedef std::shared_ptr<TriangleTexture> Ptr;

    TriangleTexture(size_t triangle_index, Vector3u triangle, unsigned width, unsigned height);
    unsigned getWidth() const;
    unsigned getHeight() const;
    void setPixel(unsigned x, unsigned y, std::uint32_t value);
    std::uint32_t getPixel(unsigned x, unsigned y) const;
    size_t getTriangleIndex() const;
    void save() const;

    double area = 0.0;
    Eigen::Vector2d texture_coordinates[3];
};

class TextureAtlas
{
    std::vector<TriangleTexture::Ptr> triangle_textures;
    MeshProject::Ptr mesh_project;
    Mesh::Ptr mesh;

public:
    typedef std::shared_ptr<TextureAtlas> Ptr;

    TextureAtlas(const MeshProject::Ptr& mesh_project, const Mesh::Ptr& mesh);

    void addTriangleTexture(const TriangleTexture::Ptr& triangle_texture);
    void build();
};
