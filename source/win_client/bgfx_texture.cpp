// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include <cstdint>
#include <fstream>
#include <iterator>
#include <vector>
#include <stdexcept>
#include "bgfx_texture.h"
#include "turbojpeg.h"

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

BgfxTexturePtr loadJpegRgbTexture(const std::string& file_name)
{
    std::ifstream input(file_name, std::ios::binary);
    std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(input), {});

    // TODO: thow an exception if texture could not be loaded or use some default texture image

    TurboJpegHandleHolder decompressor(tjInitDecompress());
    int image_width = 0;
    int image_height = 0;
    int image_jpeg_subsamp = 0;
    int image_color_space = 0;
    int error_code = tjDecompressHeader3(
        decompressor,
        &buffer[0], static_cast<unsigned long>(buffer.size()),
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
    std::vector<std::uint8_t> rgb8_buffer(static_cast<size_t>(image_width) * static_cast<size_t>(image_height) * 3);

    error_code = tjDecompress2(
        decompressor,
        &buffer[0], static_cast<unsigned long>(buffer.size()),
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
        false, 1, bgfx::TextureFormat::RGB8U, 0, texture_buffer));
    if (result_texture->isValid())
    {
        bgfx::setName(*result_texture, file_name.c_str());
    }
    return result_texture;
}
