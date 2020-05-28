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

  private:
    static constexpr int MAX_LIGHT_COUNT = 2;
    struct LightParametersConstantBuffer {
        alignas(16) DirectX::XMFLOAT3 lightPositionWorld[MAX_LIGHT_COUNT];
        alignas(16) DirectX::XMFLOAT3 viewPositionWorld;
        alignas(16) DirectX::XMFLOAT3 diffuseColor[MAX_LIGHT_COUNT];
        float attenuationConstant;
        float attenuationLinear;
        float attenuationQuadratic;
        double empty1 = 0.0f;
        double empty2 = 0.0f;
        double empty3 = 0.0f;
        double empty4 = 0.0f;
        double empty5 = 0.0f;
        double empty6 = 0.0f;
        double empty7 = 0.0f;
        double empty8 = 0.0f;
        double empty9 = 0.0f;
    };
    static LightParametersConstantBuffer lightParametersConstantBuffer;

  private:
    int number;
    mutable SolidSphere mesh;
};
