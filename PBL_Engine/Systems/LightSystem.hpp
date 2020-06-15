#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
#include "Camera.h"
#include "ECS/System.hpp"

// /////////////////////////////////////////////////////////////////// System //
ECS_SYSTEM(LightSystem) {
  public:
    // ========================================================= Behaviour == //
    // ------------------------------------- System's virtual functions -- == //
    void filters() override;
    void setup() override;
    void update(float deltaTime) override;
    void release() override;

  private:
    // ========================================================= Behaviour == //
    float oscillator(float const x);

    // ============================================================== Data == //
    float baseLightIntensity;
    float timer;
    std::shared_ptr<Camera> camera;
};

// ////////////////////////////////////////////////////////////////////////// //
