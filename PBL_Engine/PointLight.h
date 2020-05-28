#pragma once
#include "ConstantBuffers.h"
#include "Graphics.h"
#include "SolidSphere.h"

class PointLight {
  public:
    PointLight(Graphics& gfx, int number, float radius = 0.5f);
    void SpawnControlWindow() noexcept;
    void Reset() noexcept;
    void AddToBuffer(DirectX::FXMMATRIX view,
                     DirectX::XMVECTOR cameraWorldPosition);
    void Draw(Graphics& gfx) const noexcept(!IS_DEBUG);
    static void Bind(Graphics& gfx) noexcept;
    DirectX::XMFLOAT4 lightPositionWorld() const;
    void setIntensity(float intensity);

  private:
    static constexpr int MAX_LIGHT_COUNT = 8;
    struct LightParametersConstantBuffer {
        alignas(16) DirectX::XMFLOAT4 lightPositionWorld[MAX_LIGHT_COUNT];
        alignas(16) DirectX::XMFLOAT4 viewPositionWorld;
        alignas(16) DirectX::XMFLOAT4 diffuseColor[MAX_LIGHT_COUNT];
        alignas(16) DirectX::XMFLOAT4 intensity[MAX_LIGHT_COUNT / 4];
        float attenuationConstant;
        float attenuationLinear;
        float attenuationQuadratic;
    };
    static LightParametersConstantBuffer lightParametersConstantBuffer;

  private:
    int number;
    mutable SolidSphere mesh;
};
