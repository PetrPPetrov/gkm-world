// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include "common.h"
#include "mesh_project.h"
#include "mesh.h"
#include "texture.h"

class TriangleTexture
{
    Texture::Ptr texture;
    size_t triangle_index;
    Vector3u triangle;

public:
    typedef std::shared_ptr<TriangleTexture> Ptr;

    TriangleTexture(size_t triangle_index, Vector3u triangle, unsigned width, unsigned height);
    Texture::Ptr getTexture() const;
    size_t getTriangleIndex() const;
    void save() const;

    double area = 0.0;
    Eigen::Vector2d texture_coordinates[3];
};

class TextureAtlas
{
    Texture::Ptr texture_atlas;
    std::vector<TriangleTexture::Ptr> triangle_textures;
    MeshProject::Ptr mesh_project;
    Mesh::Ptr mesh;

public:
    typedef std::shared_ptr<TextureAtlas> Ptr;

    TextureAtlas(const MeshProject::Ptr& mesh_project, const Mesh::Ptr& mesh);

    void addTriangleTexture(const TriangleTexture::Ptr& triangle_texture);
    void build();
};
