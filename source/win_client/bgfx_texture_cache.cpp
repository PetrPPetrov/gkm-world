// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include <sstream>
#include <iomanip>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include "turbojpeg.h"
#include "fnv_hash.h"
#include "bgfx_texture_cache.h"

struct TurboJpegHandleHolder
{
    TurboJpegHandleHolder(tjhandle tj_handle_) : tj_handle(tj_handle_)
    {
        if (!tj_handle)
        {
            throw std::runtime_error("Can not create TurboJpeg decompressor");
        }
    }
    ~TurboJpegHandleHolder()
    {
        tjDestroy(tj_handle);
    }
    operator tjhandle() const
    {
        return tj_handle;
    }
private:
    tjhandle tj_handle;
};

BgfxTexturePtr BgfxTextureCache::loadJpegRgbTexture(const std::vector<std::uint8_t>& data, const std::string& file_name)
{
    // TODO: thow an exception if texture could not be loaded or use some default texture image

    TurboJpegHandleHolder decompressor(tjInitDecompress());
    int image_width = 0;
    int image_height = 0;
    int image_jpeg_subsamp = 0;
    int image_color_space = 0;
    int error_code = tjDecompressHeader3(
        decompressor,
        &data[0], static_cast<unsigned long>(data.size()),
        &image_width, &image_height, &image_jpeg_subsamp, &image_color_space);
    if (error_code)
    {
        throw std::runtime_error("Can not decompressed Jpeg header");
    }
    if (image_width == 0 || image_height == 0)
    {
        throw std::runtime_error("Jpeg image is empty");
    }
    if (image_width > MAX_TEXTURE_SIZE)
    {
        throw std::runtime_error("Jpeg image is too wide");
    }
    if (image_height > MAX_TEXTURE_SIZE)
    {
        throw std::runtime_error("Jpeg image is too tall");
    }
    std::vector<std::uint8_t> rgb8_buffer(static_cast<size_t>(image_width)* static_cast<size_t>(image_height) * 3);

    error_code = tjDecompress2(
        decompressor,
        &data[0], static_cast<unsigned long>(data.size()),
        &rgb8_buffer[0],
        image_width, 0, image_height, TJPF_RGB, 0);
    if (error_code)
    {
        throw std::runtime_error("Can not decompressed Jpeg image data");
    }

    const bgfx::Memory* texture_buffer = bgfx::copy(&rgb8_buffer[0], static_cast<std::uint32_t>(rgb8_buffer.size()));
    BgfxTexturePtr result_texture = makeBgfxSharedPtr(bgfx::createTexture2D(
        static_cast<std::uint16_t>(image_width),
        static_cast<std::uint16_t>(image_height),
        false, 1, bgfx::TextureFormat::RGB8, 0, texture_buffer));
    if (result_texture->isValid())
    {
        bgfx::setName(*result_texture, file_name.c_str());
    }
    return result_texture;
}

BgfxTextureCache::BgfxTextureCache(const std::string& texture_cache_dir_) : texture_cache_dir(texture_cache_dir_)
{
}

BgfxTextureInfo BgfxTextureCache::getTexture(std::uint16_t texture_id)
{
    auto find_it = id_to_texture_info.find(texture_id);
    if (find_it == id_to_texture_info.end())
    {
        BgfxTextureInfo result;
        std::stringstream file_name_in_cache;
        file_name_in_cache << texture_cache_dir << "/" << std::setw(8) << std::setfill('0') << texture_id << ".jpg";
        std::ifstream input_file(file_name_in_cache.str(), std::ifstream::binary);
        if (input_file)
        {
            input_file.seekg(0, input_file.end);
            std::size_t length = input_file.tellg();
            input_file.seekg(0, input_file.beg);

            if (length > 0)
            {
                std::vector<std::uint8_t> buffer(length);
                input_file.read(reinterpret_cast<char*>(&buffer[0]), length);
                input_file.close();

                FnvHash fnv_hash;
                fnv_hash.Update(&buffer[0], length);
                result.texture_hash = fnv_hash.getHash();
                result.texture_id = texture_id;
                result.bgfx_texture = loadJpegRgbTexture(buffer, file_name_in_cache.str());
            }
        }
        find_it = id_to_texture_info.emplace(texture_id, result).first;
    }
    return find_it->second;
}
