// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include "common.h"

class Texture
{
    std::vector<std::uint32_t> image_data;
    unsigned width;
    unsigned height;

    size_t getIndex(unsigned x, unsigned y) const;

public:
    typedef std::shared_ptr<Texture> Ptr;

    Texture(unsigned width, unsigned height);
    unsigned getWidth() const;
    unsigned getHeight() const;
    void setPixel(unsigned x, unsigned y, std::uint32_t value);
    std::uint32_t getPixel(unsigned x, unsigned y) const;
    Texture::Ptr createNewSize(unsigned new_width, unsigned new_height);

    void savePng(const char* file_name) const;
    void saveJpeg(const char* file_name) const;
    ImagePtr getQImage() const;
};
