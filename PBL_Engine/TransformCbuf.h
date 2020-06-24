#pragma once
#include <DirectXMath.h>

#include "Camera.h"
#include "ConstantBuffers.h"
#include "Renderable.h"

class TransformCbuf : public Bindable {
  private:
    struct Transforms {
        DirectX::XMMATRIX model;
        DirectX::XMMATRIX modelInverseTranspose;
        DirectX::XMMATRIX viewProj;
        DirectX::XMMATRIX lightPos;
    };

  public:
    TransformCbuf(Graphics& gfx, const Renderable& parent, UINT slot = 0u);
    void Bind(Graphics& gfx) noexcept override;

  private:
    static std::unique_ptr<VertexConstantBuffer<Transforms>> pVcbuf;
    const Renderable& parent;
};
