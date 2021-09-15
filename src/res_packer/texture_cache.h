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

struct TextureInfo {
    typedef std::shared_ptr<TextureInfo> Ptr;

    std::string file_path;
    std::uint16_t texture_id;
    std::uint64_t texture_hash;
    bool cached = true;
};

class TextureCache {
    void addCachedTexture(const std::string& file_name, std::uint16_t texture_id);
    void addNewTexture(const std::string& file_name);
    std::uint16_t getNextTextureId();

public:
    typedef std::shared_ptr<TextureCache> Ptr;

    TextureCache(const std::string& texture_cache_dir);
    void loadNewTextures(const std::string& input_model_dir);
    void updateCache();
    std::uint16_t getTextureId(const std::string& file_name) const;

private:
    const std::string texture_cache_dir;
    std::set<std::string> files_to_remove_from_cache;
    std::map<std::string, TextureInfo::Ptr> file_path_to_texture_info;
    std::unordered_map<std::uint16_t, TextureInfo::Ptr> id_to_texture_info;
    std::unordered_map<std::uint64_t, TextureInfo::Ptr> hash_to_texture_info;
    std::uint16_t next_texture_id = 1;
};
