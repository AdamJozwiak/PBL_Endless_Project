#pragma once
#include <array>

#include "Camera.h"
#include "ConstantBuffers.h"
#include "EngineAPI.hpp"
#include "FixedQueue.h"
#include "Graphics.h"
#include "SolidSphere.h"

class ENGINE_API PointLight {
  public:
    PointLight(Graphics& gfx, float radius = 0.5f);
    ~PointLight();
    void SpawnControlWindow() noexcept;
    void Reset() noexcept;
    void AddToBuffer(DirectX::FXMMATRIX view,
                     DirectX::XMVECTOR cameraWorldPosition);
    void Draw(Graphics& gfx) const noexcept(!IS_DEBUG);
    static void Bind(Graphics& gfx) noexcept;
    void AddCameras();
    DirectX::XMFLOAT4 lightPositionWorld() const;
    void setLightPositionWorld(DirectX::XMVECTOR newWorldPos);
    void setMainLightPosition(DirectX::XMVECTOR pos);
    void setIntensity(float intensity);
    float getIntensity();
    void setAttenuationC(float value);
    float getAttenuationC();
    void setAttenuationL(float value);
    float getAttenuationL();
    void setAttenuationQ(float value);
    float getAttenuationQ();
    static void initTorchNumbers();
    int getNumber();
    void setColor(DirectX::XMFLOAT4 const& color);
    DirectX::XMFLOAT4 getColor();
    std::shared_ptr<Camera> getLightCamera(int iterator);

  private:
    static constexpr int MAX_LIGHT_COUNT = 16;
    static FixedQueue<int, MAX_LIGHT_COUNT> torchNumbers;
    struct LightParametersConstantBuffer {
        alignas(16) DirectX::XMFLOAT4 lightPositionWorld[MAX_LIGHT_COUNT];
        alignas(16) DirectX::XMFLOAT4 viewPositionWorld;
        alignas(16) DirectX::XMFLOAT4 diffuseColor[MAX_LIGHT_COUNT];
        alignas(16) DirectX::XMFLOAT4 intensity[MAX_LIGHT_COUNT / 4];
        alignas(16) DirectX::XMFLOAT4 attenuationConstant[MAX_LIGHT_COUNT / 4];
        alignas(16) DirectX::XMFLOAT4 attenuationLinear[MAX_LIGHT_COUNT / 4];
        alignas(16) DirectX::XMFLOAT4 attenuationQuadratic[MAX_LIGHT_COUNT / 4];
        alignas(16) DirectX::XMFLOAT4 mainLightPosition;
    };
    static LightParametersConstantBuffer lightParametersConstantBuffer;
    float lightIntensity;
    float attenuationConstant;
    float attenuationLinear;
    float attenuationQuadratic;
    std::array<std::shared_ptr<Camera>, 6> cameras;

  private:
    int number;
    mutable SolidSphere mesh;
};
