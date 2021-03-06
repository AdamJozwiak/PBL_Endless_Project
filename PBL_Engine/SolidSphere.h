#pragma once
#include "RenderableBase.h"

class SolidSphere : public RenderableBase<SolidSphere> {
  public:
    SolidSphere(Graphics& gfx, float radius);
    void Update(float dt) noexcept override;
    void SetPos(DirectX::XMFLOAT3 pos) noexcept;
    DirectX::XMMATRIX GetTransformXM() const noexcept override;

    float scale = 1.0f;

  private:
    DirectX::XMFLOAT3 pos = {1.0f, 1.0f, 1.0f};
};