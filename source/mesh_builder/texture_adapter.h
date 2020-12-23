// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include "common.h"
#include "mesh_project.h"

class TextureAdapter
{
    Camera::Ptr camera;

public:
    typedef std::shared_ptr<TextureAdapter> Ptr;

    TextureAdapter(const Camera::Ptr& camera);

    unsigned getWidth() const;
    unsigned getHeight() const;
    std::uint32_t getPixel(unsigned x, unsigned y) const;
};
