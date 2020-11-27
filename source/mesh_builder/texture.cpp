// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include "texture.h"

size_t Texture::getIndex(unsigned x, unsigned y) const
{
    return (static_cast<size_t>(height) - 1 - y) * width + x;
}

Texture::Texture(unsigned width_, unsigned height_) : image_data(static_cast<size_t>(width_)* height_, 0xffffffff)
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

static inline int extractRed(std::uint32_t argb)
{
    return ((argb >> 16) & 0xff);
}

static inline int extractGreen(std::uint32_t argb)
{
    return ((argb >> 8) & 0xff);
}

static inline int extractBlue(std::uint32_t argb)
{
    return (argb & 0xff);
}

static inline int extractAlpha(std::uint32_t argb)
{
    return argb >> 24;
}

static inline std::uint32_t packRgb(int r, int g, int b)
{
    return 0xff000000 | ((r & 0xffu) << 16) | ((g & 0xffu) << 8) | (b & 0xffu);
}

static inline std::uint32_t packRgba(int r, int g, int b, int a)
{
    return ((a & 0xffu) << 24) | ((r & 0xffu) << 16) | ((g & 0xffu) << 8) | (b & 0xffu);
}

std::uint32_t Texture::getInterpolatedPixel(const Eigen::Vector2d& xy) const
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
    if (pixel.x() >= static_cast<double>(width - 1))
    {
        pixel.x() = static_cast<double>(width - 1);
        decrease_left = true;
    }
    if (pixel.y() >= static_cast<double>(height - 1))
    {
        pixel.y() = static_cast<double>(height - 1);
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

    const double distance_to_left = pixel.x() - left;
    const double distance_to_right = right - pixel.x();
    const double distance_to_bottom = pixel.y() - bottom;
    const double distance_to_top = top - pixel.y();

    const double lower_left_square = distance_to_left * distance_to_bottom;
    const double lower_right_square = distance_to_right * distance_to_bottom;
    const double upper_left_square = distance_to_left * distance_to_top;
    const double upper_right_square = distance_to_right * distance_to_top;

    std::uint32_t ll_pixel = getPixel(static_cast<int>(left), static_cast<int>(bottom));
    std::uint32_t lr_pixel = getPixel(static_cast<int>(right), static_cast<int>(bottom));
    std::uint32_t ul_pixel = getPixel(static_cast<int>(left), static_cast<int>(top));
    std::uint32_t ur_pixel = getPixel(static_cast<int>(right), static_cast<int>(top));

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

void Texture::savePng(const char* file_name) const
{
    QImage image(reinterpret_cast<const unsigned char*>(&image_data[0]), width, height, QImage::Format_RGB32);
    image.save(QString(file_name), "PNG");
}

void Texture::saveJpeg(const char* file_name) const
{
    QImage image(reinterpret_cast<const unsigned char*>(&image_data[0]), width, height, QImage::Format_RGB32);
    image.save(QString(file_name), "JPG");
}

ImagePtr Texture::getQImage() const
{
    return std::make_shared<QImage>(reinterpret_cast<const unsigned char*>(&image_data[0]), width, height, QImage::Format_RGB32);
}
