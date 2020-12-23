// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include "texture_adapter.h"

TextureAdapter::TextureAdapter(const Camera::Ptr& camera_) : camera(camera_)
{
}

unsigned TextureAdapter::getWidth() const
{
    return static_cast<unsigned>(cameraGetWidth(camera));
}

unsigned TextureAdapter::getHeight() const
{
    return static_cast<unsigned>(cameraGetHeight(camera));
}

std::uint32_t TextureAdapter::getPixel(unsigned x_, unsigned y_) const
{
    const int width = camera->photo_image->width();
    const int height = camera->photo_image->height();

    int x = static_cast<int>(x_);
    int y = static_cast<int>(y_);
    int res_x = x;
    int res_y = y;

    switch (camera->rotation)
    {
    default:
    case 0:
        break;
    case 90:
        res_x = width - 1 - y;
        res_y = x;
        break;
    case 180:
        res_x = width - 1 - x;
        res_y = height - 1 - y;
        break;
    case 270:
        res_x = y;
        res_y = height - 1 - x;
        break;
    }

    res_y = height - 1 - res_y;
    return camera->photo_image->pixel(res_x, res_y);
}
