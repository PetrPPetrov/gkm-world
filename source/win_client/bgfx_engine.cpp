// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include "main.h"
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

    static bool layers_initialized = false;
    if (!layers_initialized)
    {
        BgfxVertex::init();
    }

    draw_ref_info = std::make_shared<BgfxDrawRefInfo>();
    draw_ref_info->program = loadProgram();
    draw_ref_info->texture = makeBgfxSharedPtr(bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler));

    texture_cache = std::make_shared<BgfxTextureCache>(".\\tex_cache");
    model_cache = std::make_shared<BgfxModelCache>(texture_cache, ".\\res_cache", ".\\mod_cache");
    model = model_cache->getModel(1);
}

void BgfxEngine::draw()
{
    double view_x_pos = g_player_location.x_pos + sin(g_player_location.direction * GRAD_TO_RAD) * 10;
    double view_y_pos = g_player_location.y_pos + cos(g_player_location.direction * GRAD_TO_RAD) * 10;
    float view_matrix[16];
    bx::mtxLookAt(view_matrix,
        bx::Vec3(static_cast<float>(g_player_location.x_pos), static_cast<float>(g_player_location.y_pos), 1.6f),
        bx::Vec3(static_cast<float>(view_x_pos), static_cast<float>(view_x_pos), 1.6f),
        bx::Vec3(0.0f, 0.0f, 1.0f));

    float projection_matrix[16];
    bx::mtxProj(
        projection_matrix, 40.0f,
        static_cast<float>(g_window_width) / static_cast<float>(g_window_height),
        static_cast<float>(0.125f),
        static_cast<float>(1024.0f),
        bgfx::getCaps()->homogeneousDepth
    );
    bgfx::setViewTransform(0, view_matrix, projection_matrix);
    bgfx::setViewRect(0, 0, 0, g_window_width, g_window_height);

    bgfx::touch(0);
    BgfxDrawInfo draw_info;
    draw_info.ref_info = draw_ref_info;
    draw_info.state = BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_MSAA;
    drawModel(model, draw_info);
    bgfx::frame();
}

void BgfxEngine::drawModel(const BgfxModel::Ptr& model, BgfxDrawInfo draw_info)
{
    const BgfxResource::Ptr& resource = model->resource;
    for (auto& mesh : resource->meshes)
    {
        bgfx::setState(draw_info.state);
        bgfx::setTexture(0, *draw_info.ref_info->texture, *model->textures[mesh->relative_texture_id]);
        bgfx::setVertexBuffer(0, *mesh->vertex_buffer);
        bgfx::submit(0, *draw_info.ref_info->program);
    }
}
