// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <cstdint>
#include <cstddef>
#include <memory>
#include "bgfx_common.h"
#include "bgfx_shader.h"
#include "bgfx_draw_info.h"

class BgfxEngine
{
public:
    typedef std::unique_ptr<BgfxEngine> Ptr;

    BgfxEngine(std::uint32_t width, std::uint32_t height, void* native_windows_handle, bgfx::RendererType::Enum render_type);

    void draw();

private:
    BgfxEngineShutdown bgfx_engine_shutdown;

    std::uint32_t width;
    std::uint32_t height;
    std::uint32_t debug;
    std::uint32_t reset;
    BgfxDrawRefInfo::Ptr draw_ref_info;
};
