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
