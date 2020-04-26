#pragma once
#include <DirectXMath.h>

#include "ConstantBuffers.h"
#include "Mesh.h"
#include "Renderable.h"
class BonesCbuf : public Bindable {
  private:
    struct Transforms {
        DirectX::XMMATRIX BoneTransforms[256];
    };

  public:
    BonesCbuf(Graphics& gfx, Model& parent, float* animationTime,
              UINT slot = 5u);
    void Bind(Graphics& gfx) noexcept override;

  private:
    static std::unique_ptr<VertexConstantBuffer<Transforms>> pVcbuf;
    Model& parent;
    float* animationTime;
};
