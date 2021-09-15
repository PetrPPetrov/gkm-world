// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include "bgfx_common.h"

struct BgfxTextureInfo {
    BgfxTexturePtr bgfx_texture;
    std::uint16_t texture_id = 0;
    std::uint64_t texture_hash = 0;
};

class BgfxTextureCache {
    static const int MAX_TEXTURE_SIZE = 4096;
    BgfxTexturePtr loadJpegRgbTexture(const std::vector<std::uint8_t>& data, const std::string& file_name);

public:
    typedef std::shared_ptr<BgfxTextureCache> Ptr;

    BgfxTextureCache(const std::string& texture_cache_dir);
    BgfxTextureInfo getTexture(std::uint16_t texture_id);

private:
    const std::string texture_cache_dir;
    std::unordered_map<std::uint16_t, BgfxTextureInfo> id_to_texture_info;
};
