// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include <sstream>
#include <iomanip>
#include <fstream>
#include "gkm_world/fnv_hash.h"
#include "bgfx_model_cache.h"

BgfxResource::Ptr BgfxModelCache::loadGkmModelRev0BgfxResource(const std::vector<std::uint8_t>& data) {
    BgfxResource::Ptr result = std::make_shared<BgfxResource>();

    FnvHash fnv_hash;
    fnv_hash.Update(&data[0], data.size());
    result->resource_hash = fnv_hash.getHash();

    size_t index = 0;
    GkmModelRev0::Resource::Ptr resource = GkmModelRev0::loadResource(data, index);

    for (auto mesh : resource->meshes) {
        BgfxMesh::Ptr new_mesh = std::make_shared<BgfxMesh>();
        new_mesh->relative_texture_id = mesh->relative_texture_id;

        auto& vertices = mesh->vertices;
        const uint32_t vertex_count = static_cast<uint32_t>(vertices.size());
        const bgfx::Memory* vertex_buffer = bgfx::alloc(static_cast<uint32_t>(sizeof(BgfxVertex)) * vertex_count);
        auto vbo = reinterpret_cast<BgfxVertex*>(vertex_buffer->data);
        for (uint32_t i = 0; i < vertex_count; ++i) {
            vbo[i].x = vertices[i].x;
            vbo[i].y = vertices[i].y;
            vbo[i].z = vertices[i].z;
            vbo[i].u = vertices[i].u / GkmModelRev0::TEX_COORD_MULTIPLIER;
            vbo[i].v = vertices[i].v / GkmModelRev0::TEX_COORD_MULTIPLIER;
        }

        new_mesh->vertex_buffer = makeBgfxSharedPtr(bgfx::createVertexBuffer(vertex_buffer, BgfxVertex::ms_layout));
        result->meshes.push_back(new_mesh);
    }

    return result;
}

BgfxModel::Ptr BgfxModelCache::loadGkmModelRev0(const std::vector<std::uint8_t>& data) {
    BgfxModel::Ptr result = std::make_shared<BgfxModel>();

    FnvHash fnv_hash;
    fnv_hash.Update(&data[0], data.size());
    result->model_hash = fnv_hash.getHash();

    size_t index = 0;
    GkmModelRev0::Model::Ptr model = GkmModelRev0::loadModel(data, index);

    for (std::uint8_t i = 0; i < GkmModelRev0::Model::TEXTURE_COUNT; ++i) {
        result->texture_ids[i] = model->texture_ids[i];
        if (result->texture_ids[i]) {
            result->textures[i] = texture_cache->getTexture(result->texture_ids[i]).bgfx_texture;
        }
        result->resource = getResource(model->resource_id);
    }

    return result;
}

BgfxModelCache::BgfxModelCache(BgfxTextureCache::Ptr texture_cache_, const std::string& resource_cache_dir_, const std::string& model_cache_dir_)
    : resource_cache_dir(resource_cache_dir_), model_cache_dir(model_cache_dir_) {
    texture_cache = texture_cache_;
}

BgfxResource::Ptr BgfxModelCache::getResource(std::uint16_t resource_id) {
    auto find_it = id_to_resource.find(resource_id);
    if (find_it == id_to_resource.end()) {
        BgfxResource::Ptr result = std::make_shared<BgfxResource>();
        std::stringstream file_name_in_cache;
        file_name_in_cache << resource_cache_dir << "/" << std::setw(8) << std::setfill('0') << resource_id << ".gkr";
        std::ifstream input_file(file_name_in_cache.str(), std::ifstream::binary);
        if (input_file) {
            input_file.seekg(0, input_file.end);
            std::size_t length = input_file.tellg();
            input_file.seekg(0, input_file.beg);

            if (length > 0) {
                std::vector<std::uint8_t> buffer(length);
                input_file.read(reinterpret_cast<char*>(&buffer[0]), length);
                input_file.close();

                result = loadGkmModelRev0BgfxResource(buffer);
                result->resource_id = resource_id;
            }
        }
        find_it = id_to_resource.emplace(resource_id, result).first;
    }
    return find_it->second;
}

BgfxModel::Ptr BgfxModelCache::getModel(std::uint16_t model_id) {
    auto find_it = id_to_model.find(model_id);
    if (find_it == id_to_model.end()) {
        BgfxModel::Ptr result = std::make_shared<BgfxModel>();
        std::stringstream file_name_in_cache;
        file_name_in_cache << model_cache_dir << "/" << std::setw(8) << std::setfill('0') << model_id << ".gkm";
        std::ifstream input_file(file_name_in_cache.str(), std::ifstream::binary);
        if (input_file) {
            input_file.seekg(0, input_file.end);
            std::size_t length = input_file.tellg();
            input_file.seekg(0, input_file.beg);

            if (length > 0) {
                std::vector<std::uint8_t> buffer(length);
                input_file.read(reinterpret_cast<char*>(&buffer[0]), length);
                input_file.close();

                result = loadGkmModelRev0(buffer);
                result->model_id = model_id;
            }
        }
        find_it = id_to_model.emplace(model_id, result).first;
    }
    return find_it->second;
}
