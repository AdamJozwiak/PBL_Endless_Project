#pragma once
#include <DirectXMath.h>

#include "Components/Components.hpp"
#include "ConstantBuffers.h"
#include "Renderable.h"

class SkyboxCbuf : public Bindable {
  private:
    struct Transforms {
        float animationSpeed;
        float padding[3];
    };

  public:
    SkyboxCbuf(Graphics& gfx, const Renderable& parent, Skybox* skybox,
               UINT slot = 0u);
    void Bind(Graphics& gfx) noexcept override;

  private:
    static std::unique_ptr<PixelConstantBuffer<Transforms>> pPcbuf;
    const Renderable& parent;
    Skybox* pSkybox;
};
