#include "Blender.h"

#include "GraphicsThrowMacros.h"

Blender::Blender(Graphics& gfx, bool blending = false) : blending(blending) {
    INFOMAN(gfx);

    D3D11_BLEND_DESC blendDesc = {};
    auto& brt = blendDesc.RenderTarget[0];
    if (blending) {
        brt.BlendEnable = TRUE;
        brt.SrcBlend = D3D11_BLEND_SRC_ALPHA;
        brt.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        brt.BlendOp = D3D11_BLEND_OP_ADD;
        brt.SrcBlendAlpha = D3D11_BLEND_ZERO;
        brt.DestBlendAlpha = D3D11_BLEND_ZERO;
        brt.BlendOpAlpha = D3D11_BLEND_OP_ADD;
        brt.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    } else {
        brt.BlendEnable = FALSE;
        brt.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    }
    GFX_THROW_INFO(GetDevice(gfx)->CreateBlendState(&blendDesc, &pBlender));
}

void Blender::Bind(Graphics& gfx) noexcept {
    GetContext(gfx)->OMSetBlendState(pBlender.Get(), nullptr, 0xFFFFFFFFu);
}