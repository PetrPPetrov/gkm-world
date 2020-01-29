// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <cstdint>
#include <memory>
#include "bgfx/bgfx.h"
#include "bx/bx.h"
#include "bx/math.h"
#include "imgui/imgui.h"

template<class BgfxType>
struct BgfxHandleHolder
{
    BgfxHandleHolder(BgfxType bgfx_handle_) : bgfx_handle(bgfx_handle_)
    {
    }
    ~BgfxHandleHolder()
    {
        bgfx::destroy(bgfx_handle);
    }
    operator BgfxType() const
    {
        return bgfx_handle;
    }
    bool isValid() const
    {
        return bgfx::isValid(bgfx_handle);
    }
private:
    BgfxType bgfx_handle;
};

template<class BgfxType>
using BgfxUniquePtr = std::unique_ptr<BgfxHandleHolder<BgfxType>>;

template<class BgfxType>
using BgfxSharedPtr = std::shared_ptr<BgfxHandleHolder<BgfxType>>;

template<class BgfxType>
BgfxUniquePtr<BgfxType> makeBgfxUniquePtr(BgfxType bgfx_handle)
{
    return std::make_unique<BgfxHandleHolder<BgfxType>>(bgfx_handle);
}

template<class BgfxType>
BgfxSharedPtr<BgfxType> makeBgfxSharedPtr(BgfxType bgfx_handle)
{
    return std::make_shared<BgfxHandleHolder<BgfxType>>(bgfx_handle);
}

typedef BgfxSharedPtr<bgfx::VertexBufferHandle> BgfxVertexBufferPtr;
typedef BgfxSharedPtr<bgfx::IndexBufferHandle> BgfxIndexBufferPtr;
typedef BgfxSharedPtr<bgfx::ShaderHandle> BgfxShaderPtr;
typedef BgfxSharedPtr<bgfx::ProgramHandle> BgfxProgramPtr;
typedef BgfxSharedPtr<bgfx::UniformHandle> BgfxUniformPtr;
typedef BgfxSharedPtr<bgfx::TextureHandle> BgfxTexturePtr;

struct BgfxEngineShutdown
{
    ~BgfxEngineShutdown()
    {
        bgfx::shutdown();
    }
};

#pragma pack(push, 1)

struct BgfxVertex
{
    float x, y, z;
    std::int16_t u, v;

    static void init()
    {
        ms_layout
            .begin()
            .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Int16, true, true)
            .end();
    }

    static bgfx::VertexLayout ms_layout;
};

#pragma pack(pop)
