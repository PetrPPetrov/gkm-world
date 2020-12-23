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
#include "triangle_texture.h"

class TextureAtlas
{
    Texture::Ptr texture_atlas;
    MeshProject::Ptr mesh_project;
    Mesh::Ptr mesh;

public:
    typedef std::shared_ptr<TextureAtlas> Ptr;

    TextureAtlas(const MeshProject::Ptr& mesh_project, const Mesh::Ptr& mesh);

    void build();

    std::vector<TriangleTexture::Ptr> triangle_textures;
};
