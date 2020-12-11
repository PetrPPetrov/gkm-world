// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <cstdint>
#include "common.h"
#include "texture.h"

inline int extractRed(std::uint32_t argb)
{
    return ((argb >> 16) & 0xff);
}

inline int extractGreen(std::uint32_t argb)
{
    return ((argb >> 8) & 0xff);
}

inline int extractBlue(std::uint32_t argb)
{
    return (argb & 0xff);
}

inline int extractAlpha(std::uint32_t argb)
{
    return argb >> 24;
}

inline std::uint32_t packRgb(int r, int g, int b)
{
    return 0xff000000 | ((r & 0xffu) << 16) | ((g & 0xffu) << 8) | (b & 0xffu);
}

inline std::uint32_t packRgba(int r, int g, int b, int a)
{
    return ((a & 0xffu) << 24) | ((r & 0xffu) << 16) | ((g & 0xffu) << 8) | (b & 0xffu);
}

inline unsigned getWidth(const Texture* texture)
{
    return texture->getWidth();
}

inline unsigned getHeight(const Texture* texture)
{
    return texture->getHeight();
}

inline std::uint32_t getPixel(const Texture* texture, unsigned x, unsigned y)
{
    return texture->getPixel(x, y);
}

inline unsigned getWidth(const Texture::Ptr& texture)
{
    return texture->getWidth();
}

inline unsigned getHeight(const Texture::Ptr& texture)
{
    return texture->getHeight();
}

inline std::uint32_t getPixel(const Texture::Ptr& texture, unsigned x, unsigned y)
{
    return texture->getPixel(x, y);
}

inline unsigned getWidth(const ImagePtr& texture)
{
    return static_cast<unsigned>(texture->width());
}

inline unsigned getHeight(const ImagePtr& texture)
{
    return static_cast<unsigned>(texture->height());
}

inline std::uint32_t getPixel(const ImagePtr& texture, unsigned x, unsigned y)
{
    return texture->pixel(static_cast<int>(x), static_cast<int>(texture->height() - 1 - y));
}

template<class TextureType>
std::uint32_t getInterpolatedPixel(const TextureType& texture, const Eigen::Vector2d& xy)
{
    Eigen::Vector2d pixel = xy;
    bool increase_right = false;
    bool decrease_left = false;
    bool increase_top = false;
    bool decrease_bottom = false;

    if (pixel.x() <= 0.0)
    {
        pixel.x() = 0.0;
        increase_right = true;
    }
    if (pixel.y() < 0.0)
    {
        pixel.y() = 0.0;
        increase_top = true;
    }
    const double limit_x = static_cast<double>(getWidth(texture) - 1);
    if (pixel.x() >= limit_x)
    {
        pixel.x() = limit_x;
        decrease_left = true;
    }
    const double limit_y = static_cast<double>(getHeight(texture) - 1);
    if (pixel.y() >= limit_y)
    {
        pixel.y() = limit_y;
        decrease_bottom = true;
    }

    double left = floor(pixel.x());
    double right = ceil(pixel.x());
    double bottom = floor(pixel.y());
    double top = ceil(pixel.y());

    if (increase_right)
    {
        right += 1.0;
    }
    else if (decrease_left)
    {
        left -= 1.0;
    }

    if (increase_top)
    {
        top += 1.0;
    }
    else if (decrease_bottom)
    {
        bottom -= 1.0;
    }

    if (right == left)
    {
        right += 1.0;
    }
    if (top == bottom)
    {
        top += 1.0;
    }

    const double distance_to_left = pixel.x() - left;
    const double distance_to_right = right - pixel.x();
    const double distance_to_bottom = pixel.y() - bottom;
    const double distance_to_top = top - pixel.y();

    const double lower_left_square = distance_to_left * distance_to_bottom;
    const double lower_right_square = distance_to_right * distance_to_bottom;
    const double upper_left_square = distance_to_left * distance_to_top;
    const double upper_right_square = distance_to_right * distance_to_top;

    std::uint32_t ll_pixel = getPixel(texture, static_cast<unsigned>(left), static_cast<unsigned>(bottom));
    std::uint32_t lr_pixel = getPixel(texture, static_cast<unsigned>(right), static_cast<unsigned>(bottom));
    std::uint32_t ul_pixel = getPixel(texture, static_cast<unsigned>(left), static_cast<unsigned>(top));
    std::uint32_t ur_pixel = getPixel(texture, static_cast<unsigned>(right), static_cast<unsigned>(top));

    Eigen::Vector4d lower_left_pixel(extractRed(ll_pixel) / 255.0, extractGreen(ll_pixel) / 255.0, extractBlue(ll_pixel) / 255.0, extractAlpha(ll_pixel) / 255.0);
    Eigen::Vector4d lower_right_pixel(extractRed(lr_pixel) / 255.0, extractGreen(lr_pixel) / 255.0, extractBlue(lr_pixel) / 255.0, extractAlpha(lr_pixel) / 255.0);
    Eigen::Vector4d upper_left_pixel(extractRed(ul_pixel) / 255.0, extractGreen(ul_pixel) / 255.0, extractBlue(ul_pixel) / 255.0, extractAlpha(ul_pixel) / 255.0);
    Eigen::Vector4d upper_right_pixel(extractRed(ur_pixel) / 255.0, extractGreen(ur_pixel) / 255.0, extractBlue(ur_pixel) / 255.0, extractAlpha(ur_pixel) / 255.0);

    const double lower_left_weight = upper_right_square;
    const double lower_right_weight = upper_left_square;
    const double upper_left_weight = lower_right_square;
    const double upper_right_weight = lower_left_square;

    Eigen::Vector4d interpolated_pixel = lower_left_pixel * lower_left_weight + lower_right_pixel * lower_right_weight + upper_left_pixel * upper_left_weight + upper_right_pixel * upper_right_weight;

    return packRgba(
        static_cast<int>(interpolated_pixel.x() * 255.0),
        static_cast<int>(interpolated_pixel.y() * 255.0),
        static_cast<int>(interpolated_pixel.z() * 255.0),
        static_cast<int>(interpolated_pixel.w() * 255.0));
}
