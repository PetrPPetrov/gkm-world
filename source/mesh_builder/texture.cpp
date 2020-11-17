// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include "texture.h"

size_t Texture::getIndex(unsigned x, unsigned y) const
{
    return (static_cast<size_t>(height) - 1 - y) * width + x;
}

Texture::Texture(unsigned width_, unsigned height_) : image_data(static_cast<size_t>(width_)* height_)
{
    width = width_;
    height = height_;
}

unsigned Texture::getWidth() const
{
    return width;
}

unsigned Texture::getHeight() const
{
    return height;
}

void Texture::setPixel(unsigned x, unsigned y, std::uint32_t value)
{
    image_data[getIndex(x, y)] = value;
}

std::uint32_t Texture::getPixel(unsigned x, unsigned y) const
{
    return image_data[getIndex(x, y)];
}

std::uint32_t Texture::getInterpolatedPixel(double x, double y) const
{
    // TODO:
    return 0;
}
