// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <cstdint>
#include <string>
#include "bgfx_common.h"

static const int MAX_TEXTURE_SIZE = 4096;

BgfxTexturePtr loadJpegRgbTexture(const std::string& file_name);
