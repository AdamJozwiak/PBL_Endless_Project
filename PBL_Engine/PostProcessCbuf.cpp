#include "PostProcessCbuf.h"

PostProcessCbuf::PostProcessCbuf(Graphics& gfx, const Renderable& parent,
                                 float blackProportion, UINT slot)
    : parent(parent), black(blackProportion) {
    if (!pPcbuf) {
        pPcbuf = std::make_unique<PixelConstantBuffer<Transforms>>(gfx, slot);
    }
}

void PostProcessCbuf::Bind(Graphics& gfx) noexcept {
    const Transforms tf = {.blackProportion = black};
    pPcbuf->Update(gfx, tf);
    pPcbuf->Bind(gfx);
}

void PostProcessCbuf::SetBlackProportion(float blackProportion) {
    black = blackProportion;
}

std::unique_ptr<PixelConstantBuffer<PostProcessCbuf::Transforms>>
    PostProcessCbuf::pPcbuf;
