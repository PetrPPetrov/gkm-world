// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <memory>
#include <string>
#include <map>
#include <unordered_map>

struct TextureInfo
{
    typedef std::shared_ptr<TextureInfo> Ptr;

    std::string file_path;
    std::uint16_t texture_id;
};

class TextureCache
{
public:
    typedef std::shared_ptr<TextureCache> Ptr;

    std::map<std::string, TextureInfo::Ptr> file_path_to_texture_info;
    std::unordered_map<std::uint16_t, TextureInfo::Ptr> texture_id_to_texture_info;
};
