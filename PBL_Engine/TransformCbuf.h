#pragma once
#include <DirectXMath.h>

#include "ConstantBuffers.h"
#include "Renderable.h"

class TransformCbuf : public Bindable {
  private:
    struct Transforms {
        DirectX::XMMATRIX modelViewProj;
        DirectX::XMMATRIX model;
    };

  public:
    TransformCbuf(Graphics& gfx, const Renderable& parent, UINT slot = 0u);
    void Bind(Graphics& gfx) noexcept override;

  private:
    static std::unique_ptr<VertexConstantBuffer<Transforms>>
        pVcbuf;  // dynamically allocated pointer to ConstantBuffer
    const Renderable& parent;
};