#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
#include <DirectXMath.h>

#include "ECS/System.hpp"

// /////////////////////////////////////////////////////////////////// System //
ECS_SYSTEM(SoundSystem) {
  public:
    // ========================================================= Behaviour == //
    // ------------------------------------- System's virtual functions -- == //
    void filters() override;
    void setup() override;
    void update(float deltaTime) override;
    void release() override;

    // ------------------------------------------------ SoLoud wrappers -- == //
    void play(std::string const &sound, float const volume = -1.0f);
    void play3d(std::string const &sound, DirectX::XMFLOAT3 position,
                float const volume = -1.0f);
    void setListener(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 at,
                     DirectX::XMFLOAT3 up);
};

// ////////////////////////////////////////////////////////////////////////// //
