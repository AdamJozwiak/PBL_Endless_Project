#pragma once
#include <DirectXMath.h>

#include "ConstantBuffers.h"
#include "Renderable.h"

class PostProcessCbuf : public Bindable {
  private:
    struct Transforms {
        float blackProportion;
        float padding[3];
    };

  public:
    PostProcessCbuf(Graphics& gfx, const Renderable& parent, float blackProportion = 1.0f, UINT slot = 0u);
    void Bind(Graphics& gfx) noexcept override;
    void SetBlackProportion(float blackProportion);

  private:
    static std::unique_ptr<PixelConstantBuffer<Transforms>> pPcbuf;
    const Renderable& parent;
    float black;
};