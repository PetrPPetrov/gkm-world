// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include <cstddef>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include "fnv_hash.h"
#include <boost/filesystem.hpp>
#include "texture_cache.h"

static inline std::uint64_t calcTextureHash(const std::string& file_name)
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

void TextureCache::addCachedTexture(const std::string& file_name, std::uint16_t texture_id)
{
    if (texture_id == 0)
    {
        throw std::runtime_error("texture id is null");
    }
    if (texture_id_to_texture_info.find(texture_id) != texture_id_to_texture_info.end())
    {
        throw std::runtime_error(std::to_string(texture_id) + " texture hash is always present");
    }

    TextureInfo::Ptr new_texture_info = std::make_shared<TextureInfo>();
    new_texture_info->file_path = file_name;
    new_texture_info->texture_id = texture_id;
    new_texture_info->texture_hash = calcTextureHash(file_name);
    if (hash_to_texture_info.find(new_texture_info->texture_hash) != hash_to_texture_info.end())
    {
        // Has a duplicate in texture cache, we will remove it later
        files_to_remove_from_cache.emplace(file_name);
        return;
    }

    texture_id_to_texture_info.emplace(new_texture_info->texture_id, new_texture_info);
    hash_to_texture_info.emplace(new_texture_info->texture_hash, new_texture_info);

    if (next_texture_id == 0)
    {
        throw std::runtime_error("texture id overflow");
    }
}

void TextureCache::addNewTexture(const std::string& file_name)
{
    if (file_path_to_texture_info.find(file_name) != file_path_to_texture_info.end())
    {
        return; // File is already added
    }

    const std::uint64_t texture_hash = calcTextureHash(file_name);
    auto find_it_by_hash = hash_to_texture_info.find(texture_hash);
    if (find_it_by_hash != hash_to_texture_info.end())
    {
        // File is already added
        file_path_to_texture_info.emplace(file_name, find_it_by_hash->second);
        return;
    }

    TextureInfo::Ptr new_texture_info = std::make_shared<TextureInfo>();
    new_texture_info->file_path = file_name;
    new_texture_info->texture_id = getNextTextureId();
    new_texture_info->texture_hash = texture_hash;
    new_texture_info->cached = false;

    file_path_to_texture_info.emplace(new_texture_info->file_path, new_texture_info);
    texture_id_to_texture_info.emplace(new_texture_info->texture_id, new_texture_info);
    hash_to_texture_info.emplace(new_texture_info->texture_hash, new_texture_info);
}

std::uint16_t TextureCache::getNextTextureId()
{
    while (texture_id_to_texture_info.find(next_texture_id) != texture_id_to_texture_info.end())
    {
        ++next_texture_id;
        if (next_texture_id == 0)
        {
            throw std::runtime_error("texture id overflow");
        }
    }
    return next_texture_id++;
}

TextureCache::TextureCache(const std::string& texture_cache) : texture_cache_path(texture_cache)
{
    boost::filesystem::path texture_cache_path(texture_cache);
    if (!boost::filesystem::exists(texture_cache_path))
    {
        boost::filesystem::create_directories(texture_cache_path);
    }
    if (boost::filesystem::is_regular_file(texture_cache_path))
    {
        throw std::runtime_error("texture output directory is a regular file");
    }

    boost::filesystem::directory_iterator end_it;
    for (boost::filesystem::directory_iterator it(texture_cache_path); it != end_it; ++it)
    {
        if (it->path().extension() == ".jpg")
        {
            try
            {
                int texture_id = std::stoi(it->path().stem().string());
                addCachedTexture(it->path().string(), static_cast<std::uint16_t>(texture_id));
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

void TextureCache::loadNewTextures(const std::string& input_model_dir)
{
    boost::filesystem::path input_texture_path(input_model_dir);
    if (!boost::filesystem::exists(input_texture_path))
    {
        throw std::runtime_error("input model directory is not exist");
    }
    if (boost::filesystem::is_regular_file(input_texture_path))
    {
        throw std::runtime_error("input model directory is a regular file");
    }

    boost::filesystem::directory_iterator end_it;
    for (boost::filesystem::directory_iterator it(input_texture_path); it != end_it; ++it)
    {
        if (it->path().extension() == ".jpg")
        {
            addNewTexture(it->path().string());
        }
    }
}

void TextureCache::updateCache()
{
    for (auto file_to_remove : files_to_remove_from_cache)
    {
        std::cout << "removing " << file_to_remove << " as duplicated" << std::endl;
        boost::filesystem::remove(file_to_remove);
    }
    for (auto texture_info : file_path_to_texture_info)
    {
        if (!texture_info.second->cached)
        {
            std::stringstream new_file_name_in_cache;
            new_file_name_in_cache << texture_cache_path << "/" << std::setw(8) << std::setfill('0') << texture_info.second->texture_id << ".jpg";
            std::cout << "copying " << texture_info.second->file_path << " => " << new_file_name_in_cache.str() << std::endl;
            boost::filesystem::copy_file(texture_info.second->file_path, new_file_name_in_cache.str(), boost::filesystem::copy_option::overwrite_if_exists);
        }
    }
}

std::uint16_t TextureCache::getTextureId(const std::string& file_name)
{
    auto find_it_by_name = file_path_to_texture_info.find(file_name);
    if (find_it_by_name == file_path_to_texture_info.end())
    {
        return 0; // Did not find texture in cache
    }
    return find_it_by_name->second->texture_id;
}
