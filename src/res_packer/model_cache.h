// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <cstdint>
#include <array>
#include <memory>
#include <string>
#include <set>
#include <map>
#include <unordered_map>
#include "gkm_model/model.h"
#include "texture_cache.h"

struct ResourceInfo {
    typedef std::shared_ptr<ResourceInfo> Ptr;

    std::string file_path;
    std::uint16_t resource_id;
    std::uint64_t resource_hash;
    std::vector<std::uint8_t> resource_data;
    bool cached = true;
};

struct ModelInfo {
    typedef std::shared_ptr<ModelInfo> Ptr;

    std::string file_path;
    std::uint16_t model_id;
    std::uint64_t model_hash;
    std::vector<std::uint8_t> model_data;
    bool cached = true;
};

struct ResourceModelInfo {
    GkmModelRev0::Resource::Ptr resource;
    GkmModelRev0::Model::Ptr model;
};

class ModelCache {
    ResourceModelInfo loadResource(const std::string& file_name) const;
    void addResource(const ResourceInfo::Ptr& resource_info);
    std::uint16_t getNextResourceId();
    void addModel(const ModelInfo::Ptr& model_info);
    std::uint16_t getNextModelId();

    void loadCachedResources();
    void loadCachedModels();

public:
    typedef std::shared_ptr<ModelCache> Ptr;

    ModelCache(TextureCache::Ptr texture_cache, const std::string& resource_cache_dir, const std::string& model_cache_dir);
    void loadNewModels(const std::string& input_model_dir);
    void updateCache();

private:
    TextureCache::Ptr texture_cache;

    const std::string resource_cache_dir;
    std::set<std::string> files_to_remove_from_resource_cache;
    std::map<std::string, ResourceInfo::Ptr> file_path_to_resource_info;
    std::unordered_map<std::uint16_t, ResourceInfo::Ptr> id_to_resource_info;
    std::unordered_map<std::uint64_t, ResourceInfo::Ptr> hash_to_resource_info;
    std::uint16_t next_resource_id = 1;

    const std::string model_cache_dir;
    std::set<std::string> files_to_remove_from_model_cache;
    std::map<std::string, ModelInfo::Ptr> file_path_to_model_info;
    std::unordered_map<std::uint16_t, ModelInfo::Ptr> id_to_model_info;
    std::unordered_map<std::uint64_t, ModelInfo::Ptr> hash_to_model_info;
    std::uint16_t next_model_id = 1;
};
