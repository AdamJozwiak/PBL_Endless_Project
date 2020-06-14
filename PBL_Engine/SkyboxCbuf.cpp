#include "SkyboxCbuf.h"

SkyboxCbuf::SkyboxCbuf(Graphics& gfx, const Renderable& parent, Skybox* skybox,
                       UINT slot)
    : parent(parent), pSkybox(skybox) {
    if (!pPcbuf) {
        pPcbuf = std::make_unique<PixelConstantBuffer<Transforms>>(gfx, slot);
    }
}

void SkyboxCbuf::Bind(Graphics& gfx) noexcept {
    Transforms tf = {.animationSpeed = pSkybox->animationSpeed};
    pPcbuf->Update(gfx, tf);
    pPcbuf->Bind(gfx);
}

std::unique_ptr<PixelConstantBuffer<SkyboxCbuf::Transforms>> SkyboxCbuf::pPcbuf;
