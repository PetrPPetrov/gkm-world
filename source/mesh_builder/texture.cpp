// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include "task.h"
#include "bilinear_interpolation.h"
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

Texture::Ptr Texture::createNewSize(unsigned new_width, unsigned new_height)
{
    Job job(static_cast<size_t>(new_width) * new_height, "Changing size of texture...");
    Texture::Ptr new_texture = std::make_shared<Texture>(new_width, new_height);
    for (unsigned x = 0; x < new_width; ++x)
    {
        for (unsigned y = 0; y < new_height; ++y)
        {
            const double cur_x = static_cast<double>(x) / new_width * width;
            const double cur_y = static_cast<double>(y) / new_height * height;
            const std::uint32_t pixel = getInterpolatedPixel(this, Eigen::Vector2d(cur_x, cur_y));
            new_texture->setPixel(x, y, pixel);
            job.step();
        }
    }
    return new_texture;
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
