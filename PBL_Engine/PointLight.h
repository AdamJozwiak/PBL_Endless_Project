#pragma once
#include "ConstantBuffers.h"
#include "Graphics.h"
#include "SolidSphere.h"

class PointLight {
  public:
    PointLight(Graphics& gfx, float radius = 0.5f);
    void SpawnControlWindow() noexcept;
    void Reset() noexcept;
    void Draw(Graphics& gfx) const noexcept(!IS_DEBUG);
    void Bind(Graphics& gfx, DirectX::FXMMATRIX view,
              DirectX::XMVECTOR cameraWorldPosition) const noexcept;

  private:
    struct LightParametersConstantBuffer {
        alignas(16) DirectX::XMFLOAT3 lightPositionWorld;
        alignas(16) DirectX::XMFLOAT3 viewPositionWorld;
        alignas(16) DirectX::XMFLOAT3 diffuseColor;
        float attenuationConstant;
        float attenuationLinear;
        float attenuationQuadratic;
    } lightParametersConstantBuffer;

  private:
    mutable SolidSphere mesh;
    mutable PixelConstantBuffer<LightParametersConstantBuffer> cbuf;
};
