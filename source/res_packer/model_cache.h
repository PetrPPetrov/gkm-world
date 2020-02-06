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

struct ResourceInfo
{
    typedef std::shared_ptr<ResourceInfo> Ptr;

    std::string file_path;
    std::uint16_t resource_id;
    std::uint64_t resource_hash;
    bool cached = true;
};

struct ModelInfo
{
    typedef std::shared_ptr<ModelInfo> Ptr;

    std::string file_path;
    std::uint16_t model_id;
    std::uint64_t model_hash;
    bool cached = true;
};

class ResourceCache
{
    void addResource(const ResourceInfo::Ptr& resource);
    std::uint16_t getNextResourceId();

public:
    typedef std::shared_ptr<ResourceCache> Ptr;

    ResourceCache(const std::string& resource_cache);
    void loadNewResources(const std::string& input_model_dir);
    void updateCache();

private:
    const std::string resource_cache_path;
    std::set<std::string> files_to_remove_from_cache;
    std::map<std::string, ResourceInfo::Ptr> file_path_to_resource_info;
    std::unordered_map<std::uint16_t, ResourceInfo::Ptr> id_to_resource_info;
    std::unordered_map<std::uint64_t, ResourceInfo::Ptr> hash_to_resource_info;
    std::uint16_t next_resource_id = 1;
};
