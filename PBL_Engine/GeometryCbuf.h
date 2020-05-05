#pragma once
#include <DirectXMath.h>

#include "ConstantBuffers.h"
#include "Renderable.h"
#include "Camera.h"

class GeometryCbuf : public Bindable {
  private:
    struct Transforms {
        DirectX::XMMATRIX model;
        DirectX::XMMATRIX viewProj;
        DirectX::XMFLOAT4 camPos;
        float textureWidth;
        float textureHeight;
    };

  public:
    GeometryCbuf(Graphics& gfx, const Renderable& parent, Camera* camera,
                 UINT slot = 0u);
    void Bind(Graphics& gfx) noexcept override;
    void SetTextureWidth(float textureWidth);
    void SetTextureHeight(float textureHeight);

  private:
    static std::unique_ptr<GeometryConstantBuffer<Transforms>> pGcbuf;
    const Renderable& parent;
    float textureWidth = 0;
    float textureHeight = 0;
    DirectX::XMFLOAT4 pCamPos;
    Camera* pCamera;
};