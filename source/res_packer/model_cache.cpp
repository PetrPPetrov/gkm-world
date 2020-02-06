// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include <iostream>
#include "fnv_hash.h"
#include <boost/filesystem.hpp>
#include "tiny_obj_loader.h"
#include "model_cache.h"

static GkmModelRev0::Resource::Ptr loadResource(const std::string& file_name)
{
    tinyobj::ObjReader obj_reader;
    if (obj_reader.ParseFromFile(file_name))
    {
        //obj_reader.GetShapes()
    }
}

static inline std::uint64_t calcResourceHash(const std::string& file_name)
{
    std::ifstream input_file(file_name, std::ifstream::binary);
    if (input_file)
    {
        // get length of file:
        input_file.seekg(0, input_file.end);
        std::size_t length = input_file.tellg();
        input_file.seekg(0, input_file.beg);

        std::vector<std::uint8_t> buffer(length + 1);
        input_file.read(reinterpret_cast<char*>(&buffer[0]), length);
        input_file.close();

        FnvHash fnv_hash;
        fnv_hash.Update(&buffer[0], length);
        return fnv_hash.getHash();
    }
    else
    {
        throw std::runtime_error(file_name + " could not be opended");
    }
}

void ResourceCache::addResource(const ResourceInfo::Ptr& resource_info)
{
    if (resource_info->cached)
    {
        if (resource_info->resource_id == 0)
        {
            files_to_remove_from_cache.emplace(resource_info->file_path);
            return;
        }
        if (id_to_resource_info.find(resource_info->resource_id) != id_to_resource_info.end())
        {
            throw std::runtime_error(std::to_string(resource_info->resource_id) + " resource id is always present");
        }

        if (hash_to_resource_info.find(resource_info->resource_hash) != hash_to_resource_info.end())
        {
            // Has a duplicate in texture cache, we will remove it later
            files_to_remove_from_cache.emplace(resource_info->file_path);
            return;
        }

        id_to_resource_info.emplace(resource_info->resource_id, resource_info);
        hash_to_resource_info.emplace(resource_info->resource_hash, resource_info);
    }
    else
    {
        if (file_path_to_resource_info.find(resource_info->file_path) != file_path_to_resource_info.end())
        {
            return; // File is already added
        }

        auto find_it_by_hash = hash_to_resource_info.find(resource_info->resource_hash);
        if (find_it_by_hash != hash_to_resource_info.end())
        {
            // File is already added
            file_path_to_resource_info.emplace(resource_info->file_path, find_it_by_hash->second);
            return;
        }

        resource_info->resource_id = getNextResourceId();

        file_path_to_resource_info.emplace(resource_info->file_path, resource_info);
        id_to_resource_info.emplace(resource_info->resource_id, resource_info);
        hash_to_resource_info.emplace(resource_info->resource_hash, resource_info);
    }
}

std::uint16_t ResourceCache::getNextResourceId()
{
    while (id_to_resource_info.find(next_resource_id) != id_to_resource_info.end())
    {
        ++next_resource_id;
        if (next_resource_id == 0)
        {
            throw std::runtime_error("resource id overflow");
        }
    }
    return next_resource_id++;
}

ResourceCache::ResourceCache(const std::string& resource_cache) : resource_cache_path(resource_cache)
{
    boost::filesystem::path resource_cache_path(resource_cache);
    if (!boost::filesystem::exists(resource_cache_path))
    {
        boost::filesystem::create_directories(resource_cache_path);
    }
    if (boost::filesystem::is_regular_file(resource_cache_path))
    {
        throw std::runtime_error("model output directory is a regular file");
    }

    boost::filesystem::directory_iterator end_it;
    for (boost::filesystem::directory_iterator it(resource_cache_path); it != end_it; ++it)
    {
        if (it->path().extension() == ".gkr")
        {
            try
            {
                int resource_id = std::stoi(it->path().stem().string());
                ResourceInfo::Ptr resource_info = std::make_shared<ResourceInfo>();
                resource_info->resource_id = static_cast<std::uint16_t>(resource_id);
                resource_info->file_path = it->path().string();
                resource_info->resource_hash = calcResourceHash(resource_info->file_path);
                addResource(resource_info);
            }
            catch (std::invalid_argument const&)
            {
            }
            catch (std::out_of_range const&)
            {
            }
        }
    }
}

void ResourceCache::loadNewResources(const std::string& input_model_dir)
{
}

void ResourceCache::updateCache()
{
    for (auto file_to_remove : files_to_remove_from_cache)
    {
        std::cout << "removing " << file_to_remove << " as duplicated" << std::endl;
        boost::filesystem::remove(file_to_remove);
    }
    //for (auto texture_info : file_path_to_resource_info)
    //{
    //    if (!texture_info.second->cached)
    //    {
    //        std::stringstream new_file_name_in_cache;
    //        new_file_name_in_cache << texture_cache_path << "/" << std::setw(8) << std::setfill('0') << texture_info.second->texture_id << ".jpg";
    //        std::cout << "copying " << texture_info.second->file_path << " => " << new_file_name_in_cache.str() << std::endl;
    //        boost::filesystem::copy_file(texture_info.second->file_path, new_file_name_in_cache.str(), boost::filesystem::copy_option::overwrite_if_exists);
    //    }
    //}
}
