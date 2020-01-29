// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include "bgfx_engine.h"

bgfx::VertexLayout BgfxVertex::ms_layout;

BgfxEngine::BgfxEngine(std::uint32_t width_, std::uint32_t height_, void* native_windows_handle, bgfx::RendererType::Enum render_type)
{
    width = width_;
    height = height_;
    debug = BGFX_DEBUG_NONE;
    reset = BGFX_RESET_NONE;

    bgfx::Init init;
    init.type = render_type;
    init.vendorId = BGFX_PCI_ID_NONE;
    init.resolution.width = width;
    init.resolution.height = height;
    init.resolution.reset = reset;
    init.platformData.nwh = native_windows_handle;
    bgfx::init(init);

    bgfx::setDebug(debug);
    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0xbababaff, 1.0f, 0);

    BgfxVertex::init();

    draw_ref_info = std::make_shared<BgfxDrawRefInfo>();
    draw_ref_info->program = loadProgram();
}

void BgfxEngine::draw()
{
    // This dummy draw call is here to make sure that view 0 is cleared
    // if no other draw calls are submitted to view 0.
    bgfx::touch(0);
    bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_MSAA);
    bgfx::frame();
}
