// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <cstdint>
#include <string>
#include <list>
#include <unordered_map>
#include "gkm_model/model.h"
#include "bgfx_common.h"
#include "bgfx_texture_cache.h"

struct BgfxMesh
{
    typedef std::shared_ptr<BgfxMesh> Ptr;

    std::uint8_t relative_texture_id = 0;
    BgfxVertexBufferPtr vertex_buffer;
};

struct BgfxResource
{
    typedef std::shared_ptr<BgfxResource> Ptr;

    std::uint16_t resource_id = 0;
    std::uint64_t resource_hash = 0;
    std::list<BgfxMesh::Ptr> meshes;
};

struct BgfxModel
{
    typedef std::shared_ptr<BgfxModel> Ptr;

    std::uint16_t model_id = 0;
    std::uint64_t model_hash = 0;
    BgfxResource::Ptr resource;
    std::array<BgfxTexturePtr, GkmModelRev0::Model::TEXTURE_COUNT> textures = { nullptr };
    std::array<std::uint16_t, GkmModelRev0::Model::TEXTURE_COUNT> texture_ids = { 0 };
};

class BgfxModelCache
{
    BgfxResource::Ptr loadGkmModelRev0BgfxResource(const std::vector<std::uint8_t>& data);
    BgfxModel::Ptr loadGkmModelRev0(const std::vector<std::uint8_t>& data);

public:
    typedef std::shared_ptr<BgfxModelCache> Ptr;

    BgfxModelCache(BgfxTextureCache::Ptr texture_cache, const std::string& resource_cache_dir, const std::string& model_cache_dir);
    BgfxResource::Ptr getResource(std::uint16_t resource_id);
    BgfxModel::Ptr getModel(std::uint16_t model_id);

private:
    BgfxTextureCache::Ptr texture_cache;
    const std::string resource_cache_dir;
    const std::string model_cache_dir;
    std::unordered_map<std::uint16_t, BgfxResource::Ptr> id_to_resource;
    std::unordered_map<std::uint16_t, BgfxModel::Ptr> id_to_model;
};
