#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
#include <DirectXMath.h>

#include "ECS/System.hpp"

// /////////////////////////////////////////////////////////////////// System //
ECS_SYSTEM(SoundSystem) {
    // --------------------------------------------------------- Usings -- == //
    using AudioHandle = int;

  public:
    // ========================================================= Behaviour == //
    // ------------------------------------- System's virtual functions -- == //
    void filters() override;
    void setup() override;
    void update(float deltaTime) override;
    void release() override;

    // ------------------------------------------------ SoLoud wrappers -- == //
    AudioHandle play(std::string const &sound, float const volume = -1.0f);
    AudioHandle playRandomSample(std::string const &name,
                                 float const volume = -1.0f);
    AudioHandle play3d(std::string const &sound, DirectX::XMFLOAT3 position,
                float const volume = -1.0f);
    void setListener(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 at,
                     DirectX::XMFLOAT3 up);

    // ----------------------------------------------- Multiple samples -- == //
    void registerMultisampleEffect(std::string const &name,
                                   std::string const &path,
                                   std::string const &extension);
};

// ////////////////////////////////////////////////////////////////////////// //
