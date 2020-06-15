#pragma once

// ///////////////////////////////////////////////////////////////// Includes //
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
    // ============================================================== Data == //
    float lightIntensity;
    float lightFactor = -0.001f;
};

// ////////////////////////////////////////////////////////////////////////// //
